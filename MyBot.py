#!/usr/bin/env python
from ants import *
from fractions import Fraction
from random import shuffle, sample
import logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

LOW_TIME = 30
HILL_ATTACK_MAX = 10 # no more than N ants go after any given enemy hill
MAX_NOTICE_DIST = 30 # assign ants to food/missions no more than N manhattan squares away -- that
MINIMAX_DEPTH = 2 #how deep to perform minimax.  Want to keep this fairly low
ATTACK_DIST = 2 # how far away to try and sic enemy ants from
BUDDY_DIST = 6 # how far away to posse up to fight enemy ants
BEST_VAL = -10000
BEST_MOVES = []
WALLS = set()

class Ant:
    def __init__(self, loc, owner):
        self.loc = loc
        self.try_loc = None
        self.path = [] #to store an A* path to a far away goal
        self.next_loc = None # assigned in next immediate turn
        self.mission_type = None # string hill, food, or land
        self.mission_loc = None
        self.wait = 0
        self.poss_moves = []
        self.owner = owner
        
        self.best_loc = None


    # Moves an ant in loc in the direction dir. Checks for possible collisions and
    # map blocks before doing so.
    #UPDATED: ant object calls this and updates accordingly
    def do_move_direction(self, world, ant_list, direction):
        new_loc = world.destination(self.loc, direction)
        orders = [ant.next_loc for ant in ant_list if ant.next_loc != None] # list of locations that will be occupied in the next turn
        current_ant = [ant for ant in ant_list if ant.loc == new_loc]
        if ((not current_ant or current_ant[0].next_loc != None) and new_loc not in orders and new_loc not in world.my_hills() and self.path[0]==new_loc):
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
            #shuffle(f)
            cval = min(f, key = lambda x: x[1])
            current = cval[0]
            
            if current == dest:
                path = self.trace_path(came_from, dest)[1:]
                if self.mission_type == 'HILL':
                    self.path = path
                elif self.mission_type == 'EXPLORE':
                    self.path = path[:1]
                else:
                    self.path = path[:5]
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
    

    '''
       def do_attack_damage(self):
        """ Kill ants which take more than 1 damage in a turn

            Each ant deals 1/#nearby_enemy damage to each nearby enemy.
              (nearby enemies are those within the attackradius)
            Any ant with at least 1 damage dies.
            Damage does not accumulate over turns
              (ie, ants heal at the end of the battle).
        """
        damage = defaultdict(Fraction)
        nearby_enemies = {}

        # each ant damages nearby enemies
        for ant in self.current_ants.values():
            enemies = self.nearby_ants(ant.loc, self.attackradius, ant.owner)
            if enemies:
                nearby_enemies[ant] = enemies
                strenth = 10 # dot dot dot
                if ant.orders[-1] == '-':
                    strenth = 10
                else:
                    strenth = 10
                damage_per_enemy = Fraction(strenth, len(enemies)*10)
                for enemy in enemies:
                    damage[enemy] += damage_per_enemy

        # kill ants with at least 1 damage
        for ant in damage:
            if damage[ant] >= 1:
                self.kill_ant(ant)
    '''

    #Sana is a busta and steals rhymesnshit
    def evaluate_moves (self, world, attack_rad2, near_buddies, near_enemies):
        total = 0
        for ant in near_buddies:
            if near_enemies:
                for enemy in near_enemies: 
                    if (world.distance(ant.try_loc, enemy.try_loc) < attack_rad2):
                        offset = world.nearby_enemies(ant.try_loc, world.attackradius2) -  world.nearby_ants(enemy.try_loc, world.attackradius2)
                        if offset > 0:
                            total -= 1
                        elif offset < 0:
                            total += 1 
        return total
                            
                    
    def gen_attck_moves(self, world, ant_list):
        total_locs = set()
        for ant in ant_list:
            aroundMe = [world.destination(ant.loc, d) for d in ['n','s','e','w']]
            #only care about unique moves
            neighbors = [nloc for nloc in aroundMe if nloc not in WALLS and nloc not in total_locs]
            total_locs.union(neighbors)
            ant.poss_moves = neighbors
        

    def max_step(self, idx, world, attack_rad2, near_buddies, near_enemies):
        global BEST_VAL
        if world.time_remaining() < LOW_TIME:
            return False
    
        if idx < len(near_buddies):
            ant = near_buddies[idx]
            for move in ant.poss_moves:
                ant.try_loc = move
                self.max_step(idx+1, world, attack_rad2, near_buddies, near_enemies)
        else:
            value = self.min_step(0, world, attack_rad2, near_buddies, near_enemies)
            if value > BEST_VAL:
                BEST_VAL = value
                #save current best move!
                for ant in near_buddies:
                    ant.best_loc = ant.try_loc
        
    
    def min_step(self, idx, world, attack_rad2, near_buddies, near_enemies):
        if world.time_remaining() < LOW_TIME:
            return False
        if idx < len(near_enemies):
            min_val = 10000
            ant = near_enemies[idx]
            for move in ant.poss_moves:
                ant.try_loc = move
                value = self.min_step(idx+1, world, attack_rad2, near_buddies, near_enemies)
                if value < BEST_VAL:
                    return -10000
                if value < min_val:
                    min_val = value
            return min_val
        else:
            return self.evaluate_moves(world, attack_rad2, near_buddies, near_enemies)
            
    def min(self, world, index, buddies, enemies, branchsofar):
        global BEST_VAL
        if world.time_remaining() < LOW_TIME:
            #return float('inf')
            return float('-inf') # assume that this branch was terrible for us
                                 # since we did not get to evaluate it
        
        if index < len(enemies) + len(buddies):
            min_val = float('inf')
            ant = enemies[index - len(buddies)]
            
            # needs pruning duhh.. maybe not now though
            neighbs = [world.destination(ant.loc, d) for d in ['n','s','e','w']]
            neighbs.append(ant.loc)
            moves = [loc for loc in neighbs if loc not in WALLS]
            
            #if len(moves) < 1:
            #    return float('inf')
            
            for move in moves:
                #sim = list(branchsofar)
                #sim.append(move)
                #value = self.min(world, index+1, buddies, enemies, sim)
                branchsofar[index] = move
                value = self.min(world, index+1, buddies, enemies, branchsofar)
                
                if value < BEST_VAL: # Min will never choose anything greater than value, so if we already have a branch
                                     # with a better value, don't bother to evaluate other moves on this branch
                    return float('-inf')
                if value < min_val:
                    min_val = value
            return min_val
        else:
            #logging.debug(str(branchsofar))
            return self.evaluate(world, len(buddies), branchsofar) # get our resulting score from this nonsense
    
    def evaluate(self, world, n, branchsofar):
        
        # locations that our ants and the enemy ants will be in for a given move
        us = branchsofar[:n]
        them = branchsofar[n:]
        
        damage = defaultdict(Fraction)
            
        # deal damage to our ants from enemy            
        for t in them:
            targs = world.getSquaresInRadius(t, world.attackradius2).intersection(us)
            if len(targs) < 1:
                continue
            force = Fraction(10, len(targs)*10)
            for u in targs:
                damage[u] += force
        
        # deal damage to us from enemy
        for u in us:
            targs = world.getSquaresInRadius(u, world.attackradius2).intersection(them)
            if len(targs) < 1:
                continue
            force = Fraction(10, len(targs)*10)
            for t in targs:
                damage[t] += force

        # tabulate score
        score = 0.0
        for (a, d) in damage.items():
            if d >= 1: # a fatal blow was dealt
                if a in us:
                    score -= 1.1
                if a in them:
                    score += 1
        
        # The following may or may not do anything...
        """
        for u in us:
            if u in world.food_list:
                score += 1
            if damage[u] < 1 and u in world.enemy_hills():
                score += 10000
        
        for t in them:
            if damage[t] < 1 and t in world.my_hills():
                score -= 10000
        """
        return score
    
    
    
    def max(self, world, index, buddies, enemies, branchsofar):
        global BEST_VAL, BEST_MOVES
        
        if world.time_remaining() < LOW_TIME:
            return 'POOPED'
        
        if index == 0:
            BEST_MOVES = branchsofar
            BEST_VAL = float('-inf')
            branchsofar = range(len(buddies) + len(enemies))
        
        if index < len(buddies):
            ant = buddies[index]
            
            # needs pruning duhh.. maybe not now though
            neighbs = [world.destination(ant.loc, d) for d in ['n','s','e','w']]
            neighbs.append(ant.loc)
            moves = [loc for loc in neighbs if loc not in WALLS and loc not in branchsofar]
            
            # This may or may not make any sense...
            # If outnumbered, assume that moving towards the enemy is going to be a bad idea... maximize distance
            # and prune out the forward motion.
            # Reverse for when you 
            """
            if len(buddies) > len(enemies):
                moves = sorted(moves, key=lambda(x): world.distance(x, enemies[0].loc), reverse=False)
            else:
                moves = sorted(moves, key=lambda(x): world.distance(x, enemies[0].loc), reverse=True)
            """
            
            # simulate & continue
            for move in moves[:3]:
                #sim = list(branchsofar)
                #sim.append(move)
                #self.max(world, index+1, buddies, enemies, sim)
                branchsofar[index] = move
                self.max(world, index+1, buddies, enemies, branchsofar)
        else:
            value = self.min(world, index, buddies, enemies, branchsofar)
            if value > BEST_VAL:
                BEST_VAL = value
                BEST_MOVES = list(branchsofar)
    
    
    
    def group_ants(self, world, ants, enemy_locs, rad):
        logging.debug("Now we are grouping ants.. ")
        groups = []
        while (ants):
            # pick an ant randomly
            ant = sample(ants, 1)[0]
            
            #rad = 1
            squares = world.getSquaresInRadius(ant.loc, rad)
            #logging.debug('ant.loc = ' + str(ant.loc) + ' rad = ' + str(rad))
            #logging.debug('Squares = ' + str(squares))
            enemies = list(squares.intersection(enemy_locs))
            
            if not enemies:
                ants.remove(ant) # if no enemies, just continue
            else:
                ants.remove(ant)
                fr = set([ant])
                ant_locs = [ant.loc for ant in ants]
                sq = world.getSquaresInRadius(ant.loc, world.viewradius2)
                
                # get friends
                fr_locs = list(sq.intersection(ant_locs))
                for f in fr_locs[:5]:
                    friend = [aloc for aloc in ants if aloc.loc == f][0]
                    fr.add(friend)
                    ants.remove(friend)
                    
                    # get threats that new friend brings
                    sq = world.getSquaresInRadius(f, rad)
                    enemies.extend(sq.intersection(enemy_locs))
                
                # get all possib threats for the group
                en = set()
                for e in set(enemies):
                    enemy = Ant(e, 'enemy')
                    en.add(enemy)
                '''
                fr = set() # these are Ant objects
                en = set() # these are objects
                openset = list([ant]) # these are objects
                enemies = set() # these are locations 
                
                while (openset and len(fr) < 5):
                    current = openset.pop(0)
                    
                    # if we are evaluating our own ant:
                    if (current.owner == MY_ANT):
                        fr.add(current)
                        ants.remove(current)
                        
                        # get threats and all of them to group
                        threats = world.getSquaresInRadius(current.loc, rad).intersection(enemy_locs)
                        for t in threats:
                            if t not in enemies: # need to look for in location list so that the objects can compare
                                tobj = Ant(t, 'enemy')
                                openset.append(tobj)
                                en.add(tobj)
                                enemies.add(t)
                    else: #this is an enemy ant, so get the ants they are attacking! but we don't have to add all of them, just add the ants
                        # get ant this guy could hurt from remaining ants
                        ant_locs = [ant.loc for ant in ants]
                        victims = world.getSquaresInRadius(current.loc, rad).intersection(ant_locs)
                        for v in victims:
                            vic = [ant for ant in ants if ant.loc == v][0]
                            if vic not in openset:
                                openset.append(vic)
                    '''
                groups.append((fr, en))
        
        logging.debug('Returning the groups...')
        return groups
        
    def fight_ants(self, world, avail_ants): 
        #'''  
        # EXPERIMENTAL
        rad = world.attackradius2 + 4*sqrt(world.attackradius2) + 4 # (radius+2)^2
        #rad = world.attackradius2 + 6*sqrt(world.attackradius2) + 9 # (radius+3)^2
        #rad = world.viewradius2
        enemy_locs = [enemy[0] for enemy in world.enemy_ants()] # enemy locations to do intersections with
        
        # Now sort em into groups!
        # returns groups of (friends, enemies) where both are lists of Ant objs
        groups = self.group_ants(world, set(avail_ants), enemy_locs, rad)
        logging.debug('Length of groups = ' + str(len(groups)))
        # Now get best fight moves for each group!
        for group in groups:
            logging.debug('Running minimax on group of size... ' + str(len(group[0])) + ' ' + str(len(group[1])))
            global BEST_VAL
            BEST_VAL = float('-inf')
            buddies = list(group[0])
            enemies = list(group[1])
            
            ## Attack stuff goes hereee :) we can also use possib moves for good stuffs
            # shouldn't gen attck moves before simulating right? it will change per ant per turn? ehh?
            ret = self.max(world, 0, buddies, enemies, [])
            logging.debug('max returned this: ' + str(ret))
            
            moves = BEST_MOVES
            
            if moves:
                for i in range(0, len(buddies)):
                    ant = buddies[i]
                    
                    oldmission = ant.mission_loc
                    oldtype = ant.mission_type
                    oldpath = ant.path
                    
                    ant.mission_loc = moves[i]
                    ant.mission_type = 'FIGHT'
                    ant.path = []
                    if ant.mission_loc == ant.loc: # staying still was the best move
                        ant.mission_loc = None
                        ant.mission_type = None
                        ant.path = []
                        avail_ants.remove(ant)
                    elif ant.do_move_location(world, self.our_ants):
                        avail_ants.remove(ant)
                    else:
                        ant.mission_loc = oldmission
                        ant.mission_type = oldtype
                        ant.path = oldpath
                    
            else:
                logging.debug("minimax timed out or failed in someway... ")
            
            
        # END EXPERIMENTAL
        '''   
        global BEST_VAL
        BEST_VAL = float('-inf') #Because screw infinity
        enemies = set([Ant(enemy[0], enemy[1]) for enemy in world.enemy_ants()])
        attack_rad2 = world.attackradius2+2*world.attackradius2+1
        #for each of our ants, perform depth-n minimax against enemies up to 5 squares away
        
        for ant in set(avail_ants):
            #need a better way to find enemy ants in proximity to a given ant...

            #How to also remove from avail ants and enemies in one swoop? Want a list and 
            #not a set to be able to iterate over them in a tricky way
            near_enemies = [enemy for enemy in enemies if world.distance(ant.loc, enemy.loc) < attack_rad2]
            if len(near_enemies) == 0:
                continue
            
            for bad_friend in near_enemies:
                enemies.remove(bad_friend)

            near_buddies = [buddy for buddy in avail_ants if world.distance(ant.loc, buddy.loc) < BUDDY_DIST]

            #generate all the possible next moves for our/enemy ants
            self.gen_attck_moves(world, near_buddies)
            self.gen_attck_moves(world, near_enemies)
            self.max_step(0, world, attack_rad2, near_buddies, near_enemies)
            
            for ant in near_buddies:
                oldmission = ant.mission_loc
                oldtype = ant.mission_type
                oldpath = ant.path
                
                ant.mission_loc = ant.best_loc
                ant.mission_type = 'FIGHT'
                ant.path = []
                if ant.do_move_location(world, self.our_ants):
                    avail_ants.remove(ant)
                    ant.best_loc = None
                else:
                    ant.mission_loc = oldmission
                    ant.mission_type = oldtype
                    ant.path = oldpath
            #find ants nearby our ant
            #i = 0
            '''
                
            

    def hunt_food(self, world, avail_ants):
        # Get set of food locations that do not already have an ant assigned to them
        food_ants = set([ant.mission_loc for ant in self.our_ants if ant.mission_type == 'FOOD'])
        food = set(world.food()).difference(food_ants)
        
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
            if counts[hill_loc] < HILL_ATTACK_MAX:
                for ant in avail_ants: # for hills, use the full ant list b/c we want to get to the enemy hill as quickly as possible
                    d = ant.betterDist(world,hill_loc, ant.loc)
                    dist.append((d,ant,hill_loc))
        
        dist.sort()
        for (d, ant, loc) in dist:
            if ant in avail_ants and counts[loc] < HILL_ATTACK_MAX:
                oldmission = ant.mission_type
                oldloc = ant.mission_loc
                oldpath = ant.path
                
                ant.mission_type = 'HILL'
                ant.mission_loc = loc
                if ant.do_move_location(world, self.our_ants):
                    counts[loc] += 1
                    avail_ants.remove(ant)
                else:
                    ant.mission_type = oldmission
                    ant.mission_loc = oldloc
                    ant.path = oldpath
        
    
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
                

    
    def update_ant_list(self, world):
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
            
            
        # Check our ant list against the official, add any ants we missed
        ours = [ant.loc for ant in self.our_ants]
        free_agents = set(world.my_ants()).difference(ours)
        for ant_loc in free_agents:
            new_ant = Ant(ant_loc, MY_ANT)
            self.our_ants.add(new_ant)
    
    def continue_on_target(self, world, avail_ants):
        for antums in set(avail_ants):
            # Continue on target
            if antums.mission_type == "FOOD":
                if antums.mission_loc in world.food(): # check if food is still there
                    antums.do_move_location(world, self.our_ants) #if yes, continue moving that way
                    avail_ants.remove(antums)
                else: # otherwise, ABORT MISSION!
                    antums.mission_type = None
                    antums.mission_loc = None
                    antums.path = []
            elif antums.mission_type == "HILL":
                if antums.mission_loc in self.hills: # still an enemy!
                    antums.do_move_location(world, self.our_ants)
                    avail_ants.remove(antums)
                else: # otherwise, the hill is taken over
                    antums.mission_type = None
                    antums.mission_loc = None
                    antums.path = []
            elif antums.mission_type != None: # keep doin what yr doin
                antums.do_move_location(world, self.our_ants)
                avail_ants.remove(antums)
    
    def do_turn(self, world): 
        # Update our_ants list (remove dead ants, update ant locations, add newly spawned ants)
        self.update_ant_list(world)
        logging.debug('Done updating the ant list.')
        
        avail_ants = set(self.our_ants) # ants that are available at this turn
        
        # KILL ALL ANTZ
        self.fight_ants(world, avail_ants)

        # attack any hills we see
        self.hunt_hills(world, avail_ants)
        logging.debug('Done hunting for hills.')
        
        # carry on
        self.continue_on_target(world, avail_ants)
        
        # hunt for more food
        self.hunt_food(world, avail_ants)
        logging.debug('Done hunting for food.')
        
        # explore the map!
        self.explore(world, avail_ants)
        logging.debug('Done exploring.')
        
        # spread your$elves out!
        #self.bread_crumb(world, avail_ants)
        
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
