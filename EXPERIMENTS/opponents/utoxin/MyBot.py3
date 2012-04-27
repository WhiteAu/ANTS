#!/usr/bin/env python
import random
from ants import *
from math import ceil, floor
from optparse import OptionParser

# define a class with a do_turn method
# the Ants.run method will parse and update bot input
# it will also run the do_turn method for us
class MyBot:
    def __init__(self):
        # define class level variables, will be remembered between turns
        parser = OptionParser()
        parser.add_option('--debug', action="store_true", dest="debug", default=False, help="enable debugging code")
        parser.add_option('--scents', action="store", type="string", dest="scents", default="exploration,food,hill", help="which scents to output for debug")

        (options, args) = parser.parse_args()

        self.debug_flag = options.debug
        self.debug_scents = options.scents.split(",")

        self.debug_scent_colors = {'food': '0 255 0', 'hill': '255 0 0', 'attack': '127 63 0', 'flee': '127 127 0', 'exploration': '0 0 255', 'home_hill': '0 127 0'}
        self.debug_scent_dividors = {'food': 25.0, 'hill': 45.0, 'attack': 25.0, 'flee': 25.0, 'exploration': 75.0, 'home_hill': 75}

        pass

    def debug(self, output):
        if self.debug_flag == False:
            return

        import pprint
        pprint.pprint(output, sys.stderr)
        sys.stderr.flush()

    # do_setup is run once at the start of the game
    # after the bot has received the game settings
    # the ants class is created and setup by the Ants.run method
    def do_setup(self, ants):
        # initialize data structures after learning the game settings
        self.ants = ants
        self.directions = ['s','e','w','n']

        self.diffusion_map = [[{'food': 0, 'hill': 0, 'attack': 0, 'flee': 0, 'exploration': 0, 'home_hill': 0} for col in range(ants.cols)]
                              for row in range(ants.rows)]

        for col in range(ants.cols):
            for row in range(ants.rows):
                if (ants.map[row][col] == WATER):
                    self.diffusion_map[row][col]['exploration'] = 9999999

    def map_wrap(self, row, col):
        return (row % self.ants.rows, col % self.ants.cols)

    def get_scentable(self, start_row, start_col, radius = 10):
        def real_distance(x1, y1, x2, y2):
            return sqrt(pow((x2-x1), 2.0) + pow((y2-y1), 2.0))

        scentable = {}

        min_row_div = radius * -1;
        max_row_div = radius + 1;
        min_col_div = radius * -1;
        max_col_div = radius + 1;

        # Let's Try North
        for check_row in range(start_row, (start_row + min_row_div), -1):
            check_row, a = self.map_wrap(check_row, start_col)
            if self.ants.map[check_row][start_col] == WATER:
                break

            # And West
            for check_col in range(start_col, (start_col + min_col_div), -1):
                distance = real_distance(start_col, start_row, check_col, check_row)
                if distance > radius:
                    break

                check_row, check_col = self.map_wrap(check_row, check_col)
                if self.ants.map[check_row][check_col] == WATER:
                    break
                else:
                    scentable[(check_row, check_col)] = distance

            # And East
            for check_col in range(start_col, (start_col + max_col_div)):
                distance = real_distance(start_col, start_row, check_col, check_row)
                if distance > radius:
                    break

                check_row, check_col = self.map_wrap(check_row, check_col)
                if self.ants.map[check_row][check_col] == WATER:
                    break
                else:
                    scentable[(check_row, check_col)] = distance

        # Let's Try South
        for check_row in range(start_row, (start_row + max_row_div)):
            check_row, a = self.map_wrap(check_row, start_col)
            if self.ants.map[check_row][start_col] == WATER:
                break

            # And West
            for check_col in range(start_col, (start_col + min_col_div), -1):
                distance = real_distance(start_col, start_row, check_col, check_row)
                if distance > radius:
                    break

                check_row, check_col = self.map_wrap(check_row, check_col)
                if self.ants.map[check_row][check_col] == WATER:
                    break
                else:
                    scentable[(check_row, check_col)] = distance

            # And East
            for check_col in range(start_col, (start_col + max_col_div)):
                distance = real_distance(start_col, start_row, check_col, check_row)
                if distance > radius:
                    break

                check_row, check_col = self.map_wrap(check_row, check_col)
                if self.ants.map[check_row][check_col] == WATER:
                    break
                else:
                    scentable[(check_row, check_col)] = distance

        # Let's Try West
        for check_col in range(start_col, (start_col + min_col_div), -1):
            a, check_col = self.map_wrap(start_row, check_col)
            if self.ants.map[start_row][check_col] == WATER:
                break

            # And West
            for check_row in range(start_row, (start_row + min_row_div), -1):
                distance = real_distance(start_col, start_row, check_col, check_row)
                if distance > radius:
                    break

                check_row, check_col = self.map_wrap(check_row, check_col)
                if self.ants.map[check_row][check_col] == WATER:
                    break
                else:
                    scentable[(check_row, check_col)] = distance

            # And East
            for check_row in range(start_row, (start_row + max_row_div)):
                distance = real_distance(start_col, start_row, check_col, check_row)
                if distance > radius:
                    break

                check_row, check_col = self.map_wrap(check_row, check_col)
                if self.ants.map[check_row][check_col] == WATER:
                    break
                else:
                    scentable[(check_row, check_col)] = distance

        # Let's Try East
        for check_col in range(start_col, (start_col + max_col_div)):
            a, check_col = self.map_wrap(start_row, check_col)
            if self.ants.map[start_row][check_col] == WATER:
                break

            # And West
            for check_row in range(start_row, (start_row + min_row_div), -1):
                distance = real_distance(start_col, start_row, check_col, check_row)
                if distance > radius:
                    break

                check_row, check_col = self.map_wrap(check_row, check_col)
                if self.ants.map[check_row][check_col] == WATER:
                    break
                else:
                    scentable[(check_row, check_col)] = distance

            # And East
            for check_row in range(start_row, (start_row + max_row_div)):
                distance = real_distance(start_col, start_row, check_col, check_row)
                if distance > radius:
                    break

                check_row, check_col = self.map_wrap(check_row, check_col)
                if self.ants.map[check_row][check_col] == WATER:
                    break
                else:
                    scentable[(check_row, check_col)] = distance

        return scentable

    # do turn is run once per turn
    # the ants class has the game state and is updated by the Ants.run method
    # it also has several helper methods to use
    def do_turn(self, ants):
        # track all moves, prevent collisions
        orders = {}
        def do_move_direction(loc, direction):
            if not keep_running():
                return False

            new_loc = ants.destination(loc, direction)
            if (ants.unoccupied(new_loc) and new_loc not in orders):
                ants.issue_order((loc, direction))
                orders[new_loc] = loc
                return True
            else:
                return False

        def move_okay(loc):
            if (ants.unoccupied(loc) and loc not in orders and ants.passable(loc)):
                return True
            return False

        targets = {}
        def do_move_location(loc, dest):
            directions = ants.direction(loc, dest)
            for direction in directions:
                if do_move_direction(loc, direction):
                    targets[dest] = loc
                    return True
            return False

        def keep_running(factor=0.75):
            if (ants.time_remaining() > (ants.turntime - (ants.turntime * factor))):
                return True
            else:
                return False

        def diffusion():
            foods = ants.food()
            enemy_hills = ants.enemy_hills()
            my_hills = ants.my_hills()

            for col in range(ants.cols):
                for row in range(ants.rows):
                    self.diffusion_map[row][col]['food'] = 0
                    self.diffusion_map[row][col]['attack'] -= 2
                    self.diffusion_map[row][col]['flee'] = 0
                    self.diffusion_map[row][col]['hill'] -= 2
                    self.diffusion_map[row][col]['home_hill'] = 0
                    if self.diffusion_map[row][col]['hill'] < 0:
                        self.diffusion_map[row][col]['hill'] = 0

            for food_row, food_col in foods:
                if not keep_running(0.25):
                    break

                random.seed((food_row, food_col))
                food_boost = (25 - random.randint(0, 10))
                scentable = self.get_scentable(food_row, food_col, 15)
                for scent_row, scent_col in scentable.keys():
                    boost = food_boost - scentable[(scent_row, scent_col)]

                    if self.diffusion_map[scent_row][scent_col]['food'] < boost:
                        self.diffusion_map[scent_row][scent_col]['food'] = boost

            for hill in enemy_hills:
                if not keep_running(0.25):
                    break

                hill_boost = 35 - random.randint(0, 10)
                hill_row, hill_col = hill[0]
                scentable = self.get_scentable(hill_row, hill_col, 30)
                for scent_row, scent_col in scentable.keys():
                    boost = hill_boost - scentable[(scent_row, scent_col)]

                    if self.diffusion_map[scent_row][scent_col]['hill'] > boost:
                        continue

                    self.diffusion_map[scent_row][scent_col]['hill'] = boost

            my_ants = ants.my_ants()
            enemy_ants = ants.enemy_ant_dict()
            ant_count = len(my_ants)
            for hill_row, hill_col in my_hills:
                if not keep_running(0.25):
                    break

                hill_boost = 45 - random.randint(0, 10)

                scentable = self.get_scentable(hill_row, hill_col, 10)
                for scent_row, scent_col in scentable.keys():
                    boost = hill_boost - scentable[(scent_row, scent_col)]

                    if self.diffusion_map[scent_row][scent_col]['home_hill'] < boost:
                        self.diffusion_map[scent_row][scent_col]['home_hill'] = boost

                    if self.diffusion_map[scent_row][scent_col]['exploration'] > 0 and (scent_row, scent_col) not in my_ants:
                        self.diffusion_map[scent_row][scent_col]['exploration'] -= (1/float(scentable[(scent_row, scent_col)]+3))

            search_radius = int(ceil(sqrt(ants.viewradius2)) - 2)
            scent_radius = int(floor(sqrt(ants.viewradius2)) - 2)
            attack_radius = int(ceil(sqrt(ants.attackradius2)) + 3)
            flee_radius = int(ceil(sqrt(ants.attackradius2)) + 2)

            for ant_row, ant_col in enemy_ants:
                if not keep_running(0.25):
                    break

                close_enemies = 1
                close_allies = 0

                attack_boost = 23 - random.randint(0, 10)
                flee_boost = 23 - random.randint(0, 10)

                searchable = self.get_scentable(ant_row, ant_col, search_radius)
                scentable = self.get_scentable(ant_row, ant_col, scent_radius)
                attackable = self.get_scentable(ant_row, ant_col, attack_radius)

                for scent_row, scent_col in attackable.keys():
                    if (scent_row, scent_col) == (ant_row, ant_col):
                        continue

                    if (attackable[(scent_row, scent_col)] > attack_radius):
                        continue

                    if (scent_row, scent_col) in enemy_ants:
                        close_enemies += 1
                    elif (scent_row, scent_col) in my_ants:
                        close_allies += 1
                    elif (scent_row, scent_col) in my_hills:
                        close_allies += 10

                scentable = self.get_scentable(ant_row, ant_col, scent_radius)
                for scent_row, scent_col in scentable.keys():
                    if (scentable[(scent_row, scent_col)] <= attack_radius):
                        if close_allies > close_enemies:
                            boost = attack_boost - scentable[(scent_row, scent_col)]

                            if self.diffusion_map[scent_row][scent_col]['attack'] < boost:
                                self.diffusion_map[scent_row][scent_col]['attack'] = boost

                        else:
                            if (scentable[(scent_row, scent_col)] >= flee_radius):
                                boost = attack_boost - scentable[(scent_row, scent_col)]

                                if self.diffusion_map[scent_row][scent_col]['attack'] < boost:
                                    self.diffusion_map[scent_row][scent_col]['attack'] = boost

                            else:
                                boost = flee_boost - scentable[(scent_row, scent_col)]

                                if self.diffusion_map[scent_row][scent_col]['flee'] < boost:
                                    self.diffusion_map[scent_row][scent_col]['flee'] = boost

                    else:
                        boost = attack_boost - scentable[(scent_row, scent_col)]

                        if self.diffusion_map[scent_row][scent_col]['attack'] < boost:
                            self.diffusion_map[scent_row][scent_col]['attack'] = boost

        def pick_scent_direction(ant_row, ant_col, scent, flee = False):
            destinations = []
            highest = 0
            lowest = 99999

            for row in (-1, 0, 1):
                for col in (-1, 0, 1):
                    if (row == 0 and col == 0):
                        continue
                    elif (row != 0 and col != 0):
                        continue

                    dest_row = (row + ant_row) % ants.rows
                    dest_col = (col + ant_col) % ants.cols

                    if flee:
                        if (self.diffusion_map[dest_row][dest_col][scent] < lowest and move_okay((dest_row, dest_col))):
                            lowest = self.diffusion_map[dest_row][dest_col][scent]
                    else:
                        if (self.diffusion_map[dest_row][dest_col][scent] > highest and move_okay((dest_row, dest_col))):
                            highest = self.diffusion_map[dest_row][dest_col][scent]

            for row in (-1, 0, 1):
                for col in (-1, 0, 1):
                    if (row == 0 and col == 0):
                        continue
                    elif (row != 0 and col != 0):
                        continue

                    dest_row = (row + ant_row) % ants.rows
                    dest_col = (col + ant_col) % ants.cols

                    if flee:
                        if (self.diffusion_map[dest_row][dest_col][scent] <= lowest):
                            if move_okay((dest_row, dest_col)):
                                destinations.append((dest_row, dest_col))
                    else:
                        if (self.diffusion_map[dest_row][dest_col][scent] >= highest):
                            if move_okay((dest_row, dest_col)):
                                destinations.append((dest_row, dest_col))

            random.shuffle(destinations)
            for destination in destinations:
                if do_move_location((ant_row, ant_col), destination):
                    break

        diffusion()

        my_ants = ants.my_ants()

        for ant_loc in my_ants:
            # Skip ants that already have orders
            if ant_loc in orders.values():
                continue

            # First priority is to flee from dangerous ants
            elif (self.diffusion_map[ant_loc[0]][ant_loc[1]]['flee'] > 1) and keep_running(0.6):
                pick_scent_direction(ant_loc[0], ant_loc[1], 'flee', True)

            else:
                ant_scent = self.diffusion_map[ant_loc[0]][ant_loc[1]]['attack']
                hill_scent = self.diffusion_map[ant_loc[0]][ant_loc[1]]['hill']
                food_scent = self.diffusion_map[ant_loc[0]][ant_loc[1]]['food']
                home_scent = self.diffusion_map[ant_loc[0]][ant_loc[1]]['home_hill']

                # Second priority is to attack other vulnerable ants
                if (ant_scent > hill_scent) and (ant_scent > food_scent) and (ant_scent > home_scent) and keep_running(0.6):
                    # Further attack scent
                    if (ant_scent > 10) and keep_running(0.5):
                        attack_boost = ant_scent

                        hill_row, hill_col = ant_loc
                        scentable = self.get_scentable(hill_row, hill_col, 30)
                        for scent_row, scent_col in scentable.keys():
                            boost = attack_boost - scentable[(scent_row, scent_col)]

                            if self.diffusion_map[scent_row][scent_col]['attack'] > boost:
                                continue

                            self.diffusion_map[scent_row][scent_col]['attack'] = boost

                    pick_scent_direction(ant_loc[0], ant_loc[1], 'attack')

                # Attack enemy hills if scent detected, and still plenty of time
                elif (hill_scent > ant_scent) and (hill_scent > food_scent) and (hill_scent > home_scent) and keep_running(0.6):

                    # Further relay hill scent
                    if (hill_scent > 10) and keep_running(0.5):
                        hill_boost = hill_scent

                        hill_row, hill_col = ant_loc
                        scentable = self.get_scentable(hill_row, hill_col, 30)
                        for scent_row, scent_col in scentable.keys():
                            boost = hill_boost - scentable[(scent_row, scent_col)]

                            if self.diffusion_map[scent_row][scent_col]['hill'] > boost:
                                continue

                            self.diffusion_map[scent_row][scent_col]['hill'] = boost

                    # Move in direction of strongest hill scent
                    pick_scent_direction(ant_loc[0], ant_loc[1], 'hill')

                # Head towards food
                elif (food_scent > 1 and keep_running(0.75)):
                    pick_scent_direction(ant_loc[0], ant_loc[1], 'food')

                # Default action is to explore
                elif keep_running(0.75):
                    pick_scent_direction(ant_loc[0], ant_loc[1], 'exploration', True)

            # Mark exploration data for all ants
            scentable = self.get_scentable(ant_loc[0], ant_loc[1], 2)
            for scent_row, scent_col in scentable.keys():
                boost = (3 - scentable[(scent_row, scent_col)])
                self.diffusion_map[scent_row][scent_col]['exploration'] += boost

        if self.debug_flag:
            for col in range(ants.cols):
                for row in range(ants.rows):
                    for scent in self.debug_scents:
                        if self.diffusion_map[row][col][scent] >= 1:
                            scent_strength = self.diffusion_map[row][col][scent]
                            if scent_strength > self.debug_scent_dividors[scent]:
                                scent_strength = self.debug_scent_dividors[scent]
                            print("v sfc %s %.2f" % (self.debug_scent_colors[scent], (scent_strength/2.0) / self.debug_scent_dividors[scent]))
                            print("v t %s %s" % (row, col))


if __name__ == '__main__':
    import cProfile

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
        cProfile.run('Ants.run(MyBot())', 'antprof')
    except KeyboardInterrupt:
        print('ctrl-c, leaving ...')
