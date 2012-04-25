
#!/usr/bin/env python
from ants import *
from random import shuffle
import logging
logging.basicConfig(level=logging.ERROR, format='%(asctime)s - %(levelname)s - %(message)s')

LOW_TIME = 30
HILL_ATTACK_MAX = 10 # no more than N ants go after any given enemy hill
MAX_NOTICE_DIST = 30 # assign ants to food/missions no more than N manhattan squares away -- that
WALLS = set()

class Ant:
    def __init__(self, loc):
        self.loc = loc
        self.path = [] #to store an A* path to a far away goal
        self.next_loc = None # assigned in next immediate turn
        self.mission_type = None # string hill, food, or land
        self.mission_loc = None
        self.wait = 0

    # Moves an ant in loc in the direction dir. Checks for possible collisions and
    # map blocks before doing so.
    #UPDATED: ant object calls this and updates accordingly
    def do_move_direction(self, world, ant_list, direction):
        new_loc = world.destination(self.loc, direction)
        orders = [ant.next_loc for ant in ant_list if ant.next_loc != None] # list of locations that will be occupied in the next turn
        
        if (world.unoccupied(new_loc) and new_loc not in orders and new_loc not in world.my_hills() and self.path[0]==new_loc):
            world.issue_order((self.loc, direction))
            self.next_loc = self.path.pop(0)
            return True
        elif self.wait > 1: # if waiting for more than 3 turns, then make a random move
            dirs = ['n','e','s','w']
            shuffle(dirs)
            for d in dirs:
                new_loc = world.destination(self.loc, d)
                if (world.unoccupied(new_loc) and new_loc not in orders and new_loc not in world.my_hills()):
                    world.issue_order((self.loc, d))
                    self.next_loc = new_loc
                    self.path = [] # reset the path because we moved away from the path
                    return True
                    
        # No move was possible
        self.next_loc = None
        return False

    # Figures out how to move next to get closer to the destination
    # Returns true if an ant can be assigned a target, false otherwise
    def do_move_location(self, world, ant_list):
        if self.mission_type == None or self.mission_loc == None or self.next_loc != None:
            return False # whoever called this, didn't do it right!!!!!! :[
        
        if len(self.path) == 0: # the path is empty, we need to use A* to get the path
            self.get_next_five_moves(world, ant_list)

        if len(self.path) > 0:
            d = world.direction(self.loc, self.path[0])[0]
            self.do_move_direction(world, ant_list, d)
            return True
        else: 
            return False 
    
    # Runs A* and returns the next five moves
    def get_next_five_moves(self, world, ant_list):
        dest = self.mission_loc
        loc = self.loc
        
        closedset = set()
        openset = set([loc])
        came_from = {}
    
        g_score = {}
        h_score = {}
        f_score = {}
    
        g_score[loc] = 0
        h_score[loc] = self.betterDist(world, loc, dest)
        f_score[loc] = g_score[loc] + h_score[loc]
        
        while openset:
            if world.time_remaining() < LOW_TIME:
                return False
                
            f = f_score.items()
            shuffle(f)
            cval = min(f, key = lambda x: x[1])
            current = cval[0]
            
            if current == dest:
                self.path = self.trace_path(came_from, dest)[1:5]
                return True
                
            
            if f_score[current] > MAX_NOTICE_DIST and self.mission_type == 'FOOD':
                return False
            
            del f_score[current]
            openset.remove(current)
            closedset.add(current)
            
            # explore possible directions
            aroundMe = [world.destination(current, d) for d in ['n','s','e','w']]
            neighbors = [nloc for nloc in aroundMe if nloc not in WALLS and nloc not in closedset and nloc not in world.my_hills()]
            
            for neighbor in neighbors:
                if neighbor in closedset:
                    continue
    
                tentative_g_score = g_score[current] + 1
                if neighbor not in openset:
                    openset.add(neighbor)
                    h_score[neighbor] = self.betterDist(world, neighbor, dest)
                    tentative_is_better = True
                elif tentative_g_score < g_score[neighbor]:
                    tentative_is_better = True
                else:
                    tentative_is_better = False
    
                if tentative_is_better:
                    came_from[neighbor] = current
                    g_score[neighbor] = tentative_g_score
                    f_score[neighbor] = g_score[neighbor] + h_score[neighbor]
                 
        return False
    
    # More accurate heuristic for distance
    def betterDist(self, world, loc1, loc2):
        row1, col1 = loc1
        row2, col2 = loc2
        cDist = min(abs(col1 - col2), world.cols - abs(col1 - col2))
        rDist = min(abs(row1 - row2), world.rows - abs(row1 - row2))
    
        man = cDist + rDist
        if man<=1:
            return man
        
        fastestgoing = world.direction(loc1, loc2)
        destG = [world.destination(loc1,x) for x in fastestgoing]
        
        if reduce(lambda x, y: x and y, map(lambda x: x in WALLS or x in world.my_hills(), destG)): # then both of the fastest routes towards are blocked
            man += 1
            
        
        fastestcoming = world.direction(loc2, loc1)
        destC = [world.destination(loc1,x) for x in fastestcoming]
        if reduce(lambda x, y: x and y, map(lambda x: x in WALLS or x in world.my_hills(), destC)): # then both of the fastest routes once we are there are blocked
            man += 1   
        return man
        
    def trace_path(self, came_from, current_node):    
        if current_node in came_from:
            p = self.trace_path(came_from, came_from[current_node])
            p.append(current_node)
            return p
        else:
            return [current_node] 
   
# define a class with a do_turn method
# the Ants.run method will parse and update bot input
# it will also run the do_turn method for us
class MyBot:
    def __init__(self):
        self.unseen = [] # unseen locations on the map
        self.hills = [] # enemy ant hills
        self.our_ants = set() #all our ants as ant objects
        
    
    def do_setup(self,ants):
        # Add entire map to unseen locations
        for row in range(ants.rows):
            for col in range(ants.cols):
                self.unseen.append((row,col))
    
    def hunt_food(self, world, avail_ants):
        # Get set of food locations that do not already have an ant assigned to them
        food = set([floc for floc in world.food()])
        food_ants = set([ant.mission_loc for ant in self.our_ants if ant.mission_type == 'FOOD'])
        food = food.difference(food_ants)
        
        dist = []
        for food_loc in food:
            for ant in avail_ants:
                d = ant.betterDist(world,food_loc,ant.loc)
                if d < MAX_NOTICE_DIST:
                    dist.append((d, ant, food_loc))
        
        dist.sort()
        for (d, ant, loc) in dist:
            if ant in avail_ants and loc in food:
                ant.mission_type = 'FOOD'
                ant.mission_loc = loc
                if ant.do_move_location(world, self.our_ants):
                    avail_ants.remove(ant)
                    food.remove(loc)
                else:
                    ant.mission_type = None
                    ant.mission_loc = None
        '''        
        # Loop over unassigned food locs, and assign minimum distance ant to each
        min_dist = {}
        min_ant = {}
        for food_loc in food:
            min_dist[food_loc] = 10000
            min_ant[food_loc] = None
            for ant in avail_ants:
                if world.time_remaining() < LOW_TIME:
                    return

                dist = world.distance(ant.loc, food_loc)
                if dist < min_dist[food_loc] and ant not in min_ant.values():
                    min_ant[food_loc] = ant
                    min_dist[food_loc] = dist
                    
        for food_loc in food:
            if min_ant[food_loc] != None:
                ant = min_ant[food_loc]
                ant.mission_loc = food_loc
                ant.mission_type = 'FOOD'
                ant.do_move_location(world, self.our_ants)
                avail_ants.remove(ant)
        '''

        
    def hunt_hills(self, world, avail_ants):      
        #add newly discovered enemy hills to our master list!
        for hill_loc, hill_owner in world.enemy_hills():
            if hill_loc not in self.hills:
                self.hills.append(hill_loc)        
        
        targets = [ant.mission_loc for ant in self.our_ants if ant.mission_type == 'HILL']
        counts = defaultdict(int)
        dist = []
        for hill_loc in self.hills:
            counts[hill_loc] = targets.count(hill_loc)
            if counts[hill_loc] > HILL_ATTACK_MAX:
                for ant in self.our_ants: # for hills, use the full ant list b/c we want to get to the enemy hill as quickly as possible
                    d = ant.betterDist(world,hill_loc, ant.loc)
                    #if d < MAX_NOTICE_DIST:
                    dist.append((d,ant,hill_loc))
        
        dist.sort()
        reassigned = []
        for (d, ant, loc) in dist:
            if ant.loc not in reassigned and counts[loc] < HILL_ATTACK_MAX:
                old_mission = ant.mission_type
                old_path = ant.path
                old_mloc = ant.mission_loc
                
                ant.mission_type = 'HILL'
                ant.mission_loc = loc
                ant.path = []
                if ant.do_move_location(world, self.our_ants):
                    if old_mission == None:
                        avail_ants.remove(ant)
                    reassigned.append(ant.loc)
                    counts[loc] += 1
                else:
                    ant.mission_type = old_mission
                    ant.mission_loc = old_mloc
                    ant.path = old_path
        
        '''
        #for each enemy hill, assign up to HILL_ATTACK_MAX nearby ants to go after it
        for hill_loc in self.hills:  
            count = 0
            while (count < HILL_ATTACK_MAX):
                avail = set(avail_ants)
                for ant in avail:
                    if world.time_remaining() < LOW_TIME:
                        break
                    if world.distance(ant.loc, hill_loc) <= HILL_NOTICE_DIST: #only do it if it's marginally close...
                        ant.mission_loc = hill_loc
                        ant.mission_type = 'HILL'
                        avail_ants.remove(ant)
                        ant.do_move_location(world, self.our_ants)
                count += 1
        '''          
    
    # Computes how open a space is on a scale from 0 to 1 (1 being totally open/no walls seens)
    def openness(self, world, loc):
        visible = world.getVisible(loc)
        land = [1 for loc in visible if loc not in WALLS and world.unoccupied(loc)] # ? is this good
        return float(len(land))/len(visible)
    
    def explore(self, world, avail_ants): # make sure we aren't targetting walls?
        edges = set()
        for loc in set(self.unseen):
            if world.visible(loc):
                self.unseen.remove(loc)
                if world.passable(loc):
                    edges.add(loc)
                else:
                    WALLS.add(loc)
                
        # Can probs be more efficient about this -- will look into it
        dist = []
        for loc in edges:
            o = self.openness(world,loc)
            for ant in set(avail_ants):
                d = ant.betterDist(world,ant.loc,loc)
                dist.append((1 - o, d, ant, loc))
        
        for (o, d, ant, loc) in dist:
            if ant in avail_ants and loc in edges:
                ant.mission_type = 'EXPLORE'
                ant.mission_loc = loc
                if ant.do_move_location(world, self.our_ants):
                    avail_ants.remove(ant)
                    edges.remove(loc)
                else:
                    ant.mission_type = None
                    ant.mission_loc = None
    

    def bound(self, world, loc):
        return (loc[0]%world.rows, loc[1]%world.cols)

    def random_location(self, world):
        x = (random.randint(0, world.rows - 1), random.randint(0, world.cols - 1))
        #while ants.visible(x) and not ants.passable(x):
        #while not ants.passable(x) and not self.time_check(ants):
        #    x = (random.randint(0, ants.rows), random.randint(0, ants.cols))
        return x

    def bread_crumb(self, world, avail_ants):
        expansions = [(3,0),(0,3),(-3,0),(0,-3), (5,0), (0,5), (-5,0), (0,-5)]
        
        for ant in set(avail_ants):
            u = []
            for e in expansions:
                loc = self.bound(world, (ant.loc[0] + e[0], ant.loc[1] + e[1]))
                if (not world.visible(loc)) and loc not in WALLS:
                    u.append(loc)
            if len(u) != 0:
                random.shuffle(u)
                ant.mission_type = 'BREAD'
                ant.mission_loc = u[0]
                if ant.do_move_location(world, self.our_ants):
                    avail_ants.remove(ant)
                else:
                    ant.mission_type = None
                    ant.mission_loc = None
                
            #else:
            #    r = self.random_location(world)
            #    if (not world.passable(r)): #or r in world.my_hills():
            #        continue
            #    ant.mission_type = 'EXPLORE'
            #    ant.mission_loc = r
            #    ant.do_move_location(world, self.our_ants)
            #    avail_ants.remove(ant)
                

    
    def update_ant_list(self, world, avail_ants):
        sort_ants = sorted(self.our_ants, key=lambda x : x.wait, reverse=True)
        for antums in set(self.our_ants):
            # Update locations of ants that have moved
            if antums.next_loc != None:
                antums.loc = antums.next_loc
                antums.next_loc = None
                antums.wait = 0
            else: # the ant had to wait in the last turn :(
                antums.wait = antums.wait + 1
            
            
            # Remove ants that don't exist anymore
            if antums.loc not in world.my_ants():
                self.our_ants.remove(antums)
                continue
                
            # Update if we've reached our target
            if antums.loc == antums.mission_loc:
                if antums.mission_loc in self.hills:
                    self.hills.remove(antums.mission_loc) # we have taken the hill
                antums.mission_loc = None
                antums.mission_type = None
                antums.path = []
            
            
            # Continue on target
            if antums.mission_type == None:
                antums.mission_loc = None
                antums.path = []
                avail_ants.add(antums) # this is an available ant
            elif antums.mission_type == "FOOD":
                if antums.mission_loc in world.food(): # check if food is still there
                    antums.do_move_location(world, self.our_ants) #if yes, continue moving that way
                else: # otherwise, ABORT MISSION!
                    antums.mission_type = None
                    antums.mission_loc = None
                    antums.path = []    
                    avail_ants.add(antums)
            elif antums.mission_type == "HILL":
                if antums.mission_loc in self.hills: # still an enemy!
                    antums.do_move_location(world, self.our_ants)
                else: # otherwise, the hill is taken over
                    antums.mission_type = None
                    antums.mission_loc = None
                    antums.path = []
                    avail_ants.add(antums)
            else: # keep doin what yr doin
                antums.do_move_location(world, self.our_ants)
            
        # Check our ant list against the official, add any ants we missed
        ours = [ant.loc for ant in self.our_ants]
        free_agents = set(world.my_ants()).difference(ours)
        for ant_loc in free_agents:
            new_ant = Ant(ant_loc)
            self.our_ants.add(new_ant)
            avail_ants.add(new_ant)
        
    
    def do_turn(self, world): 
        avail_ants = set() # ants that are available at this turn
        
        # Update our_ants list (remove dead ants, update ant locations, add newly spawned ants)
        self.update_ant_list(world, avail_ants)
        logging.debug('Done updating the ant list.')
        
        # attack any hills we see
        self.hunt_hills(world, avail_ants)
        logging.debug('Done hunting for hills.')
        
        # hunt for more food
        self.hunt_food(world, avail_ants)
        logging.debug('Done hunting for food.')
        
        # explore the map!
        self.explore(world, avail_ants)
        logging.debug('Done exploring.')
        
        self.bread_crumb(world, avail_ants)
        
        # default move
        '''
        for ant in set(avail_ants):
            if world.time_remaining() < 10:
                break
        
            directions = ['n','e','s','w']
            shuffle(directions)
            for direction in directions:
                if ant.do_move_direction(world, self.our_ants, direction):
                    break 
        '''
    
            
if __name__ == '__main__':
    # psyco will speed up python a little, but is not needed
    try:
        import psyco
        psyco.full()
    except ImportError:
        pass
    
    try:
        # if run is passed a class with a do_turn method, it will do the work
        # this is not needed, in which case you will need to write your own
        # parsing function and your own game state class
        Ants.run(MyBot())
    except KeyboardInterrupt:
        print('ctrl-c, leaving ...')
