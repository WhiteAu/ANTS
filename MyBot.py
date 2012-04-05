#!/usr/bin/env python
from ants import *
from collections import defaultdict

# define a class with a do_turn method
# the Ants.run method will parse and update bot input
# it will also run the do_turn method for us
class MyBot:
    def __init__(self):
        # define class level variables, will be remembered between turns
        self.unseen = []
        self.hills = []
        
    
    # do_setup is run once at the start of the game
    # after the bot has received the game settings
    # the ants class is created and setup by the Ants.run method
    def do_setup(self, ants):
        # initialize data structures after learning the game settings
        #Make-it-rain strategy.  Make better!
        for row in range(ants.rows):
            for col in range(ants.cols):
                self.unseen.append((row,col))

        
    
    # do turn is run once per turn
    # the ants class has the game state and is updated by the Ants.run method
    # it also has several helper methods to use
    def do_turn(self, ants):

        #this tracks all moves made, and prevents our ants from running over each other
        orders = {}
        def do_move_direction(loc, direction):
            new_loc = ants.destination(loc,direction)
            if (ants.unoccupied(new_loc) and new_loc not in orders):
                ants.issue_order((loc, direction))
                orders[new_loc] = loc
                return True
            else:
                return False

        targets = {}
        def do_move_location(loc, dest):
            directions = ants.direction(loc, dest)
            for direction in directions:
                if do_move_direction(loc, direction):
                    targets[dest] = loc
                    return True
            return False

        # loop through all my ants and try to give them orders
        # the ant_loc is an ant location tuple in (row, col) form
        
        #can't step on own hill!
        for hill_loc in ants.my_hills():
            orders[hill_loc] = None
        
        def betterDist(loc1, loc2):
            row1, col1 = loc1
            row2, col2 = loc2
            cDist = min(abs(col1 - col2), ants.cols - abs(col1 - col2))
            rDist = min(abs(row1 - row2), ants.rows - abs(row1 - row2))
            
            man = cDist + rDist
            fastest = ants.direction(loc1, loc2)
            dests = map(lambda x : ants.destination(loc1,x),fastest)
            if not reduce(lambda x,y: x or y, map(lambda x : ants.passable(x) and x not in orders, dests)):
                man = man+1
            return man
        
        
        '''
        Currently: for every piece of food, assigns the closest available ant
        Want: smartly decide whether or not to get food. i.e., if its not doing anything/stack of goals?
        or if its convenient to do so (on the way... a way to break ties?)
        '''
        def assign_food_missions():
            ant_dist  = []
            freefood = [loc for loc in ants.food() if loc not in targets] # only look at food that does not have someone assigned to it
            freeants = [ant for ant in ants.my_ants() if ant not in targets.values()] # only consider ants with no mission
            
            for food_loc in freefood:
                for ant_loc in freeants:
                    dist = betterDist(ant_loc, food_loc)
                    ant_dist.append((dist,ant_loc,food_loc))
            ant_dist.sort()
            for dist, ant_loc, food_loc in ant_dist:
                if food_loc not in targets and ant_loc not in targets.values():
                    do_move_location(ant_loc, food_loc)

        #Attack Enemy Hills!  THis is HOW WE GET POINTS, so may be worth moving up in priority.
        def assign_hill_attack_missions():
            for hill_loc, hill_owner in ants.enemy_hills():
                if hill_loc not in self.hills:
                    self.hills.append(hill_loc)
            ant_dist = []
            freeants = [ant for ant in ants.my_ants() if ant not in targets.values()] # only consider ants with no mission
            
            for hill_loc in self.hills:
                for ant_loc in freeants:
                    if ant_loc not in orders.values():
                        dist = betterDist(ant_loc, hill_loc)
                        ant_dist.append((dist, ant_loc, hill_loc))
            ant_dist.sort()
            for dist, ant_loc, hill_loc in ant_dist:
                do_move_location(ant_loc, hill_loc)

        #explore the unknown!
        #The more you know: the [:] is a list copy shortcut.  
        #Not a Good Idea to mod the list as we loop through it so we make a copy and loop through that
        def explore_nonaimlessly():
            for loc in self.unseen[:]:
                if ants.visible(loc):
                    self.unseen.remove(loc)
            for ant_loc in ants.my_ants():
                if ant_loc not in orders.values():
                    unseen_dist = []
                    for unseen_loc in self.unseen:
                        dist = ants.distance(ant_loc, unseen_loc)
                        unseen_dist.append((dist, unseen_loc))
                    unseen_dist.sort()
                    for dist, unseen_loc in unseen_dist:
                        if do_move_location(ant_loc, unseen_loc):
                            break
            
        #unblock own hill so we can keep spawning!
        for hill_loc in ants.my_hills():
            if hill_loc in ants.my_ants() and hill_loc not in orders.values():
                for direction in ('s','e','w','n'):
                    if do_move_direction(hill_loc, direction):
                        break
        
        assign_food_missions()
        assign_hill_attack_missions()
        explore_nonaimlessly()
        
        
        # check if we still have time left to calculate more orders
        #if ants.time_remaining() < 10:
        #    break
            
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
