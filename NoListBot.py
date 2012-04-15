
#!/usr/bin/env python
from ants import *
from random import shuffle

LOW_TIME = 30
HILL_ATTACK_MAX = 6 #no more than N ants go after any given enemy hill
HILL_NOTICE_DIST = 30 #assign ants to hills no more than N manhattan squares away
class Ant:
    def __init__(self, loc):
        self.loc = loc
        self.path = [] #to store an A* path to a far away goal
        self.next_loc = None # assigned in next immediate turn
        self.mission_type = None # string hill, food, or land
        self.mission_loc = None

    # Moves an ant in loc in the direction dir. Checks for possible collisions and
    # map blocks before doing so.
    #UPDATED: ant object calls this and updates accordingly
    def do_move_direction(self, world, ant_list, direction):
        new_loc = world.destination(self.loc, direction)
        orders = [ant.next_loc for ant in ant_list if ant.next_loc != None]
        if (world.unoccupied(new_loc) and new_loc not in orders):
            world.issue_order((self.loc, direction))
            self.next_loc = self.path.pop(0)
            return True
        else:
            self.next_loc = None
            return False

    # Figures out how to move next to get closer to the destination
    # Returns true if an ant can be assigned a target, false otherwise
    
    def do_move_location(self, world, ant_list):
        # If there isn't enough time, don't bother -- MOVE TO BEFORE CALCULATING A*????????
        if world.time_remaining() < 10:
            return False
        
        if self.mission_type == None or self.mission_loc == None:
            return False # whoever called this, didn't do it right!!!!!! :[
        
        
        if  not self.path: # the path is empty, we need to use A* to get the path
            self.path = self.get_next_five_moves()

        # Send the ant along its way...
        d = world.direction(loc, self.path[0])[0]
        return self.do_move_direction(world, ant_list, d):
    
    # Runs A* and returns the next five moves
    def get_next_five_moves():
        dest = self.mission_loc
        loc = self.loc
        
        closedset = []
        openset = [loc]
        came_from = {}
    
        g_score = {}
        h_score = {}
        f_score = {}
    
        g_score[loc] = 0
        h_score[loc] = self.betterDist(world, ant_list, loc, dest)
        f_score[loc] = g_score[loc] + h_score[loc]
        
        while openset:
            current = min(f_score, key = lambda x: f_score.get(x))
    
            if current == dest:
                self.path = trace_path(came_from, dest)[1:5]

                directions = ants.direction(self.loc,self.path[0])
                return self.do_move_direction(world, ant_list, directions[0]):
            
            del f_score[current]
            openset.remove(current)
            closedset.append(current)
    
            # explore possible directions
            aroundMe = [world.destination(current, d) for d in ['n','s','e','w']]
            neighbors = [nloc for nloc in aroundMe if world.passable(nloc) and nloc not in closedset]
            
            for neighbor in neighbors:
                if neighbor in closedset:
                    continue
    
                tentative_g_score = g_score[current] + 1
                if neighbor not in openset:
                    openset.append(neighbor)
                    h_score[neighbor] = self.betterDist(world, orders, neighbor, dest)
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
    def betterDist(self, world, ant_list, loc1, loc2):
        row1, col1 = loc1
        row2, col2 = loc2
        cDist = min(abs(col1 - col2), world.cols - abs(col1 - col2))
        rDist = min(abs(row1 - row2), world.rows - abs(row1 - row2))
    
        man = cDist + rDist
        if man<=1:
            return man
        
        orders = [ant.next_loc for ant in ant_list if ant.next_loc != None]
        fastest = world.direction(loc1, loc2)
        dests = map(lambda x : world.destination(loc1,x),fastest)
        if not reduce(lambda x,y: x or y, map(lambda x : world.passable(x) and x not in orders, dests)):
            man = man + 1
        return man    
   
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
                if (ants.passable((row,col))):
                    self.unseen.append((row,col))
    
    def hunt_food(self, world, avail_ants):
        # Get set of food locations that do not already have an ant assigned to them
        food = set([floc for floc in world.food()])
        food_ants = set([ant.mission_loc for ant in self.our_ants if ant.mission == 'FOOD'])
        food = food.difference(food_ants)
                
        # Loop over unassigned food locs, and assign minimum distance ant to each
        min_dist = {}
        min_ant = {}
        for food_loc in food:
            min_dist[food_loc] = 10000
            min_ant[food_loc] = None
            for ant in avail_ants:
                if world.time_remaining() < LOW_TIME:
                    self.out.write('We hit the break in food loc.\n')
                    self.finish_turn()
                    self.out.flush()
                    break

                dist = world.distance(ant.loc, food_loc)
                if dist < min_dist[food_loc] and ant not in min_ant.values():
                    min_ant[food_loc] = ant
                    min_dist[food_loc] = dist

        
    def hunt_hills(self, world, avail_ants):      
        #add newly discovered enemy hills to our master list!
        for hill_loc, hill_owner in world.enemy_hills():
            if hill_loc not in self.hills:
                self.hills.append(hill_loc)        
        
        #for each enemy hill, assign up to HILL_ATTACK_MAX nearby ants to go after it
        for hill_loc in self.hills:    
            avail = set(avail_ants)
            count = 0
            while (count < HILL_ATTACK_MAX):
                for ant in avail:
                    if world.time_remaining() < LOW_TIME:
                        break
                    if world.distance(ant.loc, hill_loc) <= HILL_NOTICE_DIST: #only do it if it's marginally close...
                        ant.mission_loc = hill_loc
                        ant.mission_type = 'HILL'
                        avail_ants.remove(ant)
                        self.do_move_location(world, avail_ants)
                count += 1
                    
    
    
    def explore(self, world, avail_ants): # make sure we aren't targetting walls?
        for loc in self.unseen[:]:
            if world.visible(loc):
                self.unseen.remove(loc)
        for ant_loc in world.my_ants():
            if world.time_remaining() < LOW_TIME:
                    self.out.write('We hit the break in explore.\n')
                    world.finish_turn()
                    self.out.flush()
                    break
            
            if ant_loc not in list(self.targets.values() + self.hill_targets.values() + self.food_targets.values()):
            #if ant_loc.mission is None:
                unseen_dist = []
                for unseen_loc in self.unseen:
                    if world.time_remaining() < LOW_TIME:
                        self.out.write('We hit the 2nd break in explore.\n')
                        world.finish_turn()
                        self.out.flush()
                        break
                    if world.passable(unseen_loc):
                        dist = world.distance(ant_loc, unseen_loc)
                        unseen_dist.append((dist, unseen_loc))
                unseen_dist.sort()
                for dist, unseen_loc in unseen_dist:
                    if self.do_move_location(world, orders, ant_loc, unseen_loc, 'LAND'):
                        break
    
    def update_ant_list(self, world, avail_ants):
        for antums in set(self.our_ants):
            
            # Remove ants that don't exist anymore
            if antums.next_loc != None:
                antums.loc = antums.next_loc
                antums.next_loc = None
            
            if world.map[antums.loc[0]][antums.loc[1]] == DEAD:
                    our_ants.remove(antums)
            if antums.loc == antum.mission_loc:
                antums.mission_loc = None
                antums.mission_type = None
            
            # Continue on target
            if antums.mission_type == None:
                    avail_ants.add(antums) # this is an available ant
            elif antums.mission_type == "FOOD":
                if antums.mission_loc in world.food(): # check if food is still there
                    antums.do_move_location(ants, self.our_ants) #if yes, continue moving that way
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
            
        
        for hill_loc in world.my_hills():
            #add new ants as they get spawned in
            if hill_loc in world.my_ants():
                new_ant = Ant(hill_loc)
                self.our_ants.add(new_ant) #add the new ant to our list!
    
    
    def do_turn(self, world): 
        avail_ants = set() # ants that are available at this turn
        
        # Update our_ants list (remove dead ants, update ant locations, add newly spawned ants)
        self.update_ant_list(world, avail_ants)

        # attack any hills we see
        self.hunt_hills(world, avail_ants)

        # hunt for more food
        self.hunt_food(world, avail_ants)
        
        # explore the map!
        self.explore(world, avail_ants)
        
        # default move
        '''
        for ant in self.avail_ants:
            if ants.time_remaining() < 10:
                break
        
            if ant not in list(self.targets.values() + self.hill_targets.values() + self.food_targets.values()):
            
                directions = ['n','e','s','w']
                shuffle(directions)
                for direction in directions:
                    if self.do_move_direction(ants, orders, ant, direction):
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
