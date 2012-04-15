
#!/usr/bin/env python
from ants import *
from random import shuffle


class Ant:
    def __init__(self, loc):
        self.loc = loc
        self.next_loc = None
        self.path = [] #to store an A* path to a far away goal
        self.mission = None
        #self.out = open('meow.txt, w')

     # Moves an ant in loc in the direction dir. Checks for possible collisions and
    # map blocks before doing so.
    #UPDATED: ant object calls this and updates accordingly
    '''
    def do_move_direction(self, bot, world, orders, direction):
        print 'loc is', self.loc,'\n'
        
        new_loc = world.destination(self.loc, direction)
        if (world.unoccupied(new_loc) and new_loc not in orders and loc not in orders.values()):
            world.issue_order((loc, direction))
            orders[new_loc] = loc
            self.next_loc = new_loc
            return True
        else:
            return False
    '''

    # Figures out how to move next to get closer to the destination
    # Returns true if an ant can be assigned a target, false otherwise
    '''
    def do_move_location(self, bot, world, orders, dest, target_type):
        if world.time_remaining() < 10:
            return False
        
        if (world.distance(self.loc, dest) == 1): #then there is no reason to run A*/no blockage possible
            d = world.direction(loc, dest)[0]
            if self.do_move_direction(world, orders, d):
                if target_type == 'FOOD':
                    self.mission = 'FOOD'
                    bot.food_targets[dest] = self.loc
                elif target_type == 'HILL':
                    self.mission = 'HILL'
                    bot.hill_targets[dest] = self.loc
                else:
                    self.mission = 'MOVE' #maybe should be more verbose?
                    bot.targets[dest] = self.loc
                return True
            else:
                self.mission = None
                return False
        
        closedset = []
        openset = [self.loc]
        came_from = {}
    
        g_score = {}
        h_score = {}
        f_score = {}
    
        g_score[loc] = 0
        h_score[loc] = self.betterDist(world, orders, self.loc, dest)
        f_score[loc] = g_score[self.loc] + h_score[self.loc]
        
        while openset:
            current = min(f_score, key = lambda x: f_score.get(x))
    
            if current == dest:
                self.path = trace_path(came_from, dest)
                if len(path) < 2:
                    self.mission = None
                    return False
                directions = ants.direction(self.loc,path[1])
                if self.do_move_direction(ants, orders, loc, directions[0]):
                    if target_type == 'FOOD':
                        self.mission = 'FOOD'
                        bot.food_targets[dest] = self.loc
                    elif target_type == 'HILL':
                        ants.mission = 'HILL'
                        bot.hill_targets[dest] = self.loc
                    else:
                        ants.mission = 'MOVE'
                        bot.targets[dest] = self.loc
                    return True
                else:
                    self.mission = None
                    return False
    
            
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
       
        self.mission = None            
        return False
        '''
    # More accurate heuristic for distance
    '''
    def betterDist(self, bot, world, orders, loc1, loc2):
        row1, col1 = loc1
        row2, col2 = loc2
        cDist = min(abs(col1 - col2), world.cols - abs(col1 - col2))
        rDist = min(abs(row1 - row2), world.rows - abs(row1 - row2))
    
        man = cDist + rDist
        if man<=1:
            return man
    
        fastest = world.direction(loc1, loc2)
        dests = map(lambda x : world.destination(loc1,x),fastest)
        if not reduce(lambda x,y: x or y, map(lambda x : world.passable(x) and x not in bot.orders, bot.dests)):
            man = man
        return man    
  '''   
# define a class with a do_turn method
# the Ants.run method will parse and update bot input
# it will also run the do_turn method for us
class MyBot:
    def __init__(self):
        self.unseen = [] # unseen locations on the map
        self.hills = [] # enemy ant hills
        self.our_ants = [] #all our ants that aren't assigned to other stuff.  Cutting down list comprehension stuff
        self.ant_objs = set() #all our ants as ant objects
        self.food_targets = {} # targets that are food 
        self.hill_targets = {} # targets that are enemy hills
        self.targets = {} # targets that are an empty piece of land
        #self.out = open('meow.txt','w')
    
    
    def do_setup(self,ants):
        # Add entire map to unseen locations
        for row in range(ants.rows):
            for col in range(ants.cols):
                if (ants.passable((row,col))):
                    self.unseen.append((row,col))
    
    
    # Moves an ant in loc in the direction dir. Checks for possible collisions and
    # map blocks before doing so.
    def do_move_direction(self, ant, world, orders, loc, direction):
        new_loc = world.destination(loc, direction)
        if (world.unoccupied(new_loc) and new_loc not in orders and loc not in orders.values()):
            world.issue_order((loc, direction))
            orders[new_loc] = loc
            return True
        else:
            return False
    
    
   
              
    # More accurate heuristic for distance
    def betterDist(self, ants, orders, loc1, loc2):
        row1, col1 = loc1
        row2, col2 = loc2
        cDist = min(abs(col1 - col2), ants.cols - abs(col1 - col2))
        rDist = min(abs(row1 - row2), ants.rows - abs(row1 - row2))
    
        man = cDist + rDist
        if man<=1:
            return man
    
        fastest = ants.direction(loc1, loc2)
        dests = map(lambda x : ants.destination(loc1,x),fastest)
        if not reduce(lambda x,y: x or y, map(lambda x : ants.passable(x) and x not in orders, dests)):
            man = man
        return man    
     
     
    def trace_path(self, came_from, current_node):    
        if current_node in came_from:
            p = self.trace_path(came_from, came_from[current_node])
            p.append(current_node)
            return p
        else:
            return [current_node]
    #'''   
    # Figures out how to move next to get closer to the destination
    # Returns true if an ant can be assigned a target, false otherwise
    def do_move_location(self, ant, world, orders, loc, dest, target_type):
        if world.time_remaining() < 10:
            return False
        
        if (world.distance(loc, dest) == 1): # then there is no reason to run A*/no blockage possible
            d = world.direction(loc, dest)[0]
            if self.do_move_direction(world, orders, loc, d):
                if target_type == 'FOOD':
                    ant.mission = 'FOOD'
                    self.food_targets[dest] = loc
                elif target_type == 'HILL':
                    ant.mission = 'HILL'
                    self.hill_targets[dest] = loc
                else:
                    ant.mission = 'MOVE' #maybe should be more verbose?
                    self.targets[dest] = loc
                return True
            else:
                ant.mission = None
                return False
        
        closedset = []
        openset = [loc]
        came_from = {}
    
        g_score = {}
        h_score = {}
        f_score = {}
    
        g_score[loc] = 0
        h_score[loc] = self.betterDist(world, orders, loc, dest)
        f_score[loc] = g_score[loc] + h_score[loc]
        
        while openset:
            current = min(f_score, key = lambda x: f_score.get(x))
    
            if current == dest:
                path = self.trace_path(came_from, dest)
                if len(path) < 2:
                    world.mission = None
                    return False
                directions = world.direction(loc,path[1])
                if self.do_move_direction(world, orders, loc, directions[0]):
                    if target_type == 'FOOD':
                        ant.mission = 'FOOD'
                        self.food_targets[dest] = loc
                    elif target_type == 'HILL':
                        ant.mission = 'HILL'
                        self.hill_targets[dest] = loc
                    else:
                        ant.mission = 'MOVE'
                        self.targets[dest] = loc
                    return True
                else:
                    ant.mission = None
                    return False
    
            
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
       
        ant.mission = None            
        return False
    
    
    # Updates the targets to reflect when an ant moves
    # target lists are dicts of locations and ants assigned to them
    def update_targets(self, orders, targets):
        for (tar_loc, ant) in targets.items():
            if ant.loc in orders.values(): # this means the ant it was assigned to has moved
                if ant.loc == tar_loc: # target has been reached!
                    del targets[tar_loc]
                    ant.mission = None
                else: # not there yet, but closer!
                    new_loc = [nloc for nloc in orders if orders[nloc] == ant.loc][0] # get ant's new location
                    targets[tar_loc] = new_loc
       
    
    
    def hunt_food(self, world, orders):
        food = [floc for floc in world.food() if floc not in self.food_targets]
        antz = [aloc for aloc in self.ant_objs if aloc not in list(self.food_targets.values() + self.hill_targets.values())]
        #antz = [aloc for aloc in world.my_ants() if aloc not in list(self.food_targets.values() + self.hill_targets.values())]
        
        
        ant_dist = []
        for food_loc in food:
            for ant in antz:
                if world.time_remaining() < 10:
                    self.out.write('We hit the break in food loc.\n')
                    self.finish_turn()
                    self.out.flush()
                    break
                dist = world.distance(ant.loc, food_loc)
                ant_dist.append((dist, ant, food_loc))
        ant_dist.sort()
        for dist, ant, food_loc in ant_dist:
            #Because of our ant_loc mission check above
            if food_loc not in self.food_targets:
                self.do_move_location(world, orders, ant, food_loc, 'FOOD')
        
    
    def hunt_hills(self, world, orders):
        targs = list(self.targets.values() + self.hill_targets.values() + self.food_targets.values())
        antz = [aloc for aloc in self.ant_objs if aloc not in targs]
        #antz = [aloc for aloc in world.my_ants() if aloc not in targs]
        #ob_ants = [antums for antums in self.ant_objs if antums.mission is None]

        for hill_loc, hill_owner in world.enemy_hills():
            if hill_loc not in self.hills:
                self.hills.append(hill_loc)        
        
        for hill_loc in self.hills:    
            self.out.write('WE FOUND A HILLLLLLLlllll\n')
            #self.out.flush()
            for ant in antz:
            #for antums in ob_ants:
                if world.time_remaining() < 10:
                    self.out.write('We hit the break in hunt hills.\n')
                    self.finish_turn()
                    self.out.flush()
                    break
                
                #self.out.write('assigning ants to kill enemy hill!\n')
                #self.out.flush()
                if ant.loc not in targs:
                    self.do_move_location(world, orders, ant, hill_loc, 'HILL')
                    #antums.do_move_location(world, orders, ant_loc, hill_loc, 'HILL')
    
    
    def explore(self, world, orders): # make sure we aren't targetting walls?
        for loc in self.unseen[:]:
            if world.visible(loc):
                self.unseen.remove(loc)
        for ant_loc in world.my_ants():
            if world.time_remaining() < 10:
                    self.out.write('We hit the break in explore.\n')
                    world.finish_turn()
                    self.out.flush()
                    break
            
            if ant_loc not in list(self.targets.values() + self.hill_targets.values() + self.food_targets.values()):
            #if ant_loc.mission is None:
                unseen_dist = []
                for unseen_loc in self.unseen:
                    if world.time_remaining() < 10:
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
    
    def do_turn(self, ants): 
        orders = {} # tracks what moves have been made
      
        #get off my lawn!
        #get rid of old ants, otherwise update locations (old loc <- new loc)
        for antums in self.ant_objs:
            if ants.map[antums.new_loc[0]][antums.new_loc[1]] == ants.DEAD:
                ant_objs.remove(antums)
                print "\nremoved an ant!\n"
            else:
                antums.lo2c = antums.new_loc
                antums.new_loc = None
                print "\nadded an ant at our hill!\n"
        
        #so add a new ant IF a new ant spawned there.
        for hill_loc in ants.my_hills():
            orders[hill_loc] = None
            #add new ants as they get spawned in
            if ants.map[hill_loc[0]][hill_loc[1]] == ants.MY_ANT:
                new_ant = Ant(hill_loc)
                self.ant_objs.add(new_ant) #add the new ant to our list!
                self.out.write('We added a new ant on our hill.\n')
                self.out.flush()
        

        #NOTE: these propagation fn's need to get switched out with path stuff.
        # don't forget about the old targets!
        for (tar_loc, ant) in self.food_targets.items():
            if tar_loc in ants.food():
                self.do_move_location(ants, orders, ant, tar_loc, 'FOOD')
            else:
                del self.food_targets[tar_loc]
                ant.mission = None
        
        for (tar_loc, ant) in self.targets.items():
            if tar_loc not in self.unseen: # place has now been seen
                del self.targets[tar_loc]
                ant.mission = None
            else:
                self.do_move_location(ants, orders, ant, tar_loc, 'LAND')
            
        for (tar_loc, ant) in self.hill_targets.items():
            if tar_loc not in ants.my_hills(): # still an enemy hill
                self.do_move_location(ants, orders, ant, tar_loc, 'HILL')
            else: # otherwise it has been taken over 
                del self.hill_targets[tar_loc]
                self.hills.remove(tar_loc)
                ant.mission = None
        

        # attack any hills we see
        self.hunt_hills(ants, orders)

        # hunt for more food
        self.hunt_food(ants, orders)
        
        # explore the map!
        #self.explore(ants, orders)
        
        # default move
        
        for ant in self.ant_objs:
        #for ant_loc in ants.my_ants():
            print "derp\n"
            if ants.time_remaining() < 10:
                break
        
            if ant not in list(self.targets.values() + self.hill_targets.values() + self.food_targets.values()):
            
                directions = ['n','e','s','w']
                shuffle(directions)
                for direction in directions:
                    if self.do_move_direction(ants, orders, ant, direction):
                        break 
                        

        '''
        for ant_loc in self.ant_objs:
            if ants.time_remaining() < 10:
                break
        
            if ant_loc not in list(self.targets.values() + self.hill_targets.values() + self.food_targets.values()):
            #if ant_loc.mission is None:
                directions = ['n','e','s','w']
                shuffle(directions)
                for direction in directions:
                    if self.do_move_direction(ants, orders, ant_loc, direction):
                        break #used to be break but that doesn't make sense...
        '''


        self.update_targets(orders, self.food_targets)
        self.update_targets(orders, self.hill_targets)
        self.update_targets(orders, self.targets)
        
        
    
            
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
