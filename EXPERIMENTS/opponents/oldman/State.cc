#include "State.h"

using namespace std;

//constructor
State::State()
{
    gameover = 0;
    turn = 0;
};

//destructor
State::~State()
{
    bug.close();
};

//sets the state up
void State::setup()
{
	bug.open("./debug.txt");
    grid = vector<vector<Square> >(rows, vector<Square>(cols, Square()));

	for (int i = 0; i < rows; i++) {
		territory_map.push_back(vector<int>(cols, UNKNOWN));
		water_blocking_penalties.push_back(vector<int>(cols, 0));
	}

	for (int i = 0; i < rows; i++) 
		for (int j = 0; j < cols; j++) {
			unvisited_map.insert(Location(i, j));
		}

	total_num_grids = 0;
	grid_length = viewradius + 2; // * 2; // / 2;
	for (int i = 0; i < rows; i += grid_length) {
		for (int j = 0; j < cols; j += grid_length) {
			unvisited_grid.insert(Location(i, j));
			all_grid_locs.insert(Location(i,j));
			total_num_grids++;
		}
	}

	// Initialize distance rings from distance 1 to distance 10
	map<LocationOffset, int> distance_map;
	map<LocationOffset, int>::iterator it;
	map<LocationOffset, int> curr_dis_map;
	distance_map[LocationOffset(0,0)] = 1;
	curr_dis_map[LocationOffset(0,0)] = 1;
	for (int i = 0; i < MAX_RING_DISTANCE; i++) {
		distance_rings.push_back(vector<LocationOffset>());
		map<LocationOffset, int> new_dis_map;
		for (it = curr_dis_map.begin(); it != curr_dis_map.end(); it++) {
			for (int d = 0; d < TDIRECTIONS; d++) {
				LocationOffset offset(it->first.row_offset + DIRECTIONS[d][0], 
									  it->first.col_offset + DIRECTIONS[d][1]);

				if (distance_map.find(offset) == distance_map.end()) {
					distance_rings[i].push_back(offset);
					distance_map[offset] = 1;
					new_dis_map[offset] = 1;
				}
			}
		}
		curr_dis_map = new_dis_map;
	}
};

//resets all non-water squares to land and clears the bots ant vector
void State::reset()
{
    myAnts.clear();
    enemyAnts.clear();
    myHills.clear();
    enemyHills.clear();
    food.clear();
	deadAnts.clear();
    for(int row=0; row<rows; row++)
        for(int col=0; col<cols; col++)
            if(!grid[row][col].isWater)
                grid[row][col].reset();
};

//
void State::UpdateState() {
	if (turn == 1) {
		my_original_hills.insert(myHills.begin(), myHills.end());
		my_remained_hills.insert(myHills.begin(), myHills.end());
	}

	// Update detected enemy hill information
	for (int i = 0; i < enemyHills.size(); i++) {
		detected_enemy_hills.insert(enemyHills[i]);
		// Add the symmetric location too
		/*int row_offset = enemyHills[i].row - my_original_hills.begin()->row;
		int col_offset = enemyHills[i].col - my_original_hills.begin()->col;
		Location loc = getLocation(*my_original_hills.begin(), -row_offset, -col_offset);
		if (my_original_hills.find(loc) == my_original_hills.end())
			detected_enemy_hills.insert(loc);

		loc = getLocation(enemyHills[i], row_offset, col_offset);
		if (my_original_hills.find(loc) == my_original_hills.end())
			detected_enemy_hills.insert(loc);*/
	}

	
	set<Location> new_detected_enemy_hills;
	set<Location>::iterator it;
	for (it = detected_enemy_hills.begin(); it != detected_enemy_hills.end(); it++) {
		if (grid[it->row][it->col].isHill || !grid[it->row][it->col].isVisible) {
			new_detected_enemy_hills.insert(*it);
		}
	}
	detected_enemy_hills = new_detected_enemy_hills;

	// update my remained hills
	set<Location> new_my_remained_hills;
	for (it = my_remained_hills.begin(); it != my_remained_hills.end(); it++) {
		if (grid[it->row][it->col].isHill || !grid[it->row][it->col].isVisible) {
			new_my_remained_hills.insert(*it);
		}
	}
	my_remained_hills = new_my_remained_hills;


	// Update food information
	for (int i = 0; i < food.size(); i++) {
		detected_food.insert(food[i]);
	}

	set<Location> new_food_set;
	for (it = detected_food.begin(); it != detected_food.end(); it++) {
		if (IsFood(*it) || !grid[it->row][it->col].isVisible) 
			new_food_set.insert(*it);
	}
	detected_food = new_food_set;


	// Update water information
	int size = water.size();
	for (int i = 0; i < size; i++) {
		if (territory_map[water[i].row][water[i].col] != WATER) {
			for (int d = 0; d < TDIRECTIONS; d++) {
				Location loc = getLocation(water[i], d);
				water_blocking_penalties[loc.row][loc.col] += WATER_BLOCKING_PENALTY;

				if (water_blocking_penalties[loc.row][loc.col] == WATER_BLOCKING_PENALTY * 3)
					territory_map[loc.row][loc.col] = WATER;
			}
			territory_map[water[i].row][water[i].col] = WATER;
		}
	}

	// Calculate bonus values for all the food
	tile_bonus_values = vector< vector<int> >(rows, vector<int>(cols, 0));
	size = food.size();
	for (int i = 0; i < size; i++) {
		for (int dis = 0; dis < MAX_RING_DISTANCE; dis++) {
			vector<LocationOffset>::iterator it;
			for (it = distance_rings[dis].begin(); it != distance_rings[dis].end(); it++) {
				Location loc = getLocation(food[i], it->row_offset, it->col_offset);
				tile_bonus_values[loc.row][loc.col] += (MAX_RING_DISTANCE - dis) * FOOD_BONUS_UNIT;
			}
		}
	}

	// Calculate bonus values for all enemy hills
	size = enemyHills.size();
	for (int i = 0; i < size; i++) {
		tile_bonus_values[enemyHills[i].row][enemyHills[i].col] += (MAX_RING_DISTANCE + 1)* ENEMY_HILL_BONUS_UNIT;
		for (int dis = 0; dis < MAX_RING_DISTANCE; dis++) {
			vector<LocationOffset>::iterator it;
			for (it = distance_rings[dis].begin(); it != distance_rings[dis].end(); it++) {
				Location loc = getLocation(enemyHills[i], it->row_offset, it->col_offset);
				tile_bonus_values[loc.row][loc.col] += (MAX_RING_DISTANCE - dis) * ENEMY_HILL_BONUS_UNIT;
			}
		}
	}

	// Update visited and unvisited map
	set<Location> new_unvisited_map;
	set<Location>::iterator tmp_it;
	for (it = unvisited_map.begin(); it != unvisited_map.end(); it++) {
		if (grid[it->row][it->col].isVisible) {
			visited_map.insert(*it);
			if ( (tmp_it = unvisited_grid.find(*it)) != unvisited_grid.end()) {
				visited_grid.insert(*it);
				unvisited_grid.erase(tmp_it);
			}
		}
		else {
			new_unvisited_map.insert(*it);
		}
	}
	unvisited_map = new_unvisited_map;

	// Update invisible grids
	invisible_grid.clear();
	for (it = all_grid_locs.begin(); it != all_grid_locs.end(); it++) {
		if (!grid[it->row][it->col].isVisible)
			invisible_grid.insert(*it);
	}

	grid_sentinel_map.clear();
}


void State::SetSentinel(Location grid_loc, int wander_dir) {
	grid_sentinel_map[grid_loc] = grid_loc;

	/*int row_pos = (ant_loc.row + grid_length / 2) / grid_length;
	int col_pos = (ant_loc.col + grid_length / 2) / grid_length;

	Location closest_grid = getLocation(Location(0,0), row_pos * grid_length, col_pos * grid_length);
	if (grid_sentinel_map.find(closest_grid) == grid_sentinel_map.end()) {
		grid_sentinel_map[closest_grid] = ant_loc;
	}
	else {
		int xy_dis = xyDistance(ant_loc, closest_grid);
		if (xyDistance(ant_loc, closest_grid) <= xyDistance(grid_sentinel_map[closest_grid], closest_grid))
			grid_sentinel_map[closest_grid] = ant_loc;
	}*/
}


//
Location State::GetClosestUnvisitedGridLocation(Location& loc, int wander_dir) {
	int row_pos = (loc.row + grid_length / 2) / grid_length;
	int col_pos = (loc.col + grid_length / 2) / grid_length;
	Location closest_grid = getLocation(Location(0,0), row_pos * grid_length, col_pos * grid_length);

	if (unvisited_grid.find(closest_grid) != unvisited_grid.end() && grid_sentinel_map.find(closest_grid) == grid_sentinel_map.end()) {
		while (territory_map[closest_grid.row][closest_grid.col] == WATER) {
			closest_grid = getLocation(closest_grid, WANDER_DIRECTIONS[wander_dir][0], WANDER_DIRECTIONS[wander_dir][1]);
		}
		return closest_grid;
	}
	else {
		int num_row_grids = rows / grid_length / 2;
		int num_col_grids = cols / grid_length / 2;
		int num_grids = num_row_grids > num_col_grids ? num_row_grids : num_col_grids;

		Location target_loc = NULL_LOCATION;
		Location loc;
		for (int i = 1; i <= num_grids; i++) 
			for (int j = 0; j <= i; j++) {
				loc = getLocation(closest_grid, grid_length * i, grid_length * j);
				if (unvisited_grid.find(loc) != unvisited_grid.end()  && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * i, grid_length * -j);
				if (unvisited_grid.find(loc) != unvisited_grid.end()  && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * -i, grid_length * j);
				if (unvisited_grid.find(loc) != unvisited_grid.end()  && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * -i, grid_length * -j);
				if (unvisited_grid.find(loc) != unvisited_grid.end()  && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * j, grid_length * i);
				if (unvisited_grid.find(loc) != unvisited_grid.end()  && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * -j, grid_length * i);
				if (unvisited_grid.find(loc) != unvisited_grid.end()  && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * j, grid_length * -i);
				if (unvisited_grid.find(loc) != unvisited_grid.end()  && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * -j, grid_length * -i);
				if (unvisited_grid.find(loc) != unvisited_grid.end()  && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}
			}
	}

	return closest_grid;
}


//
Location State::GetClosestInvisibleGridLocation(Location& loc, int wander_dir) {
	int row_pos = (loc.row + grid_length / 2) / grid_length;
	int col_pos = (loc.col + grid_length / 2) / grid_length;
	Location closest_grid = getLocation(Location(0,0), row_pos * grid_length, col_pos * grid_length);

	if (invisible_grid.find(closest_grid) != invisible_grid.end() && grid_sentinel_map.find(closest_grid) == grid_sentinel_map.end()) {
		while (territory_map[closest_grid.row][closest_grid.col] == WATER) {
			closest_grid = getLocation(closest_grid, WANDER_DIRECTIONS[wander_dir][0], WANDER_DIRECTIONS[wander_dir][1]);
		}
		return closest_grid;
	}
	else {
		int num_row_grids = rows / grid_length / 2;
		int num_col_grids = cols / grid_length / 2;
		int num_grids = num_row_grids > num_col_grids ? num_row_grids : num_col_grids;

		Location target_loc = NULL_LOCATION;
		Location loc;
		for (int i = 1; i <= num_grids; i++) 
			for (int j = 0; j <= i; j++) {
				loc = getLocation(closest_grid, grid_length * i, grid_length * j);
				if (invisible_grid.find(loc) != invisible_grid.end() && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * i, grid_length * -j);
				if (invisible_grid.find(loc) != invisible_grid.end() && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * -i, grid_length * j);
				if (invisible_grid.find(loc) != invisible_grid.end() && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * -i, grid_length * -j);
				if (invisible_grid.find(loc) != invisible_grid.end() && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * j, grid_length * i);
				if (invisible_grid.find(loc) != invisible_grid.end() && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * -j, grid_length * i);
				if (invisible_grid.find(loc) != invisible_grid.end() && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * j, grid_length * -i);
				if (invisible_grid.find(loc) != invisible_grid.end() && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}

				loc = getLocation(closest_grid, grid_length * -j, grid_length * -i);
				if (invisible_grid.find(loc) != invisible_grid.end() && grid_sentinel_map.find(loc) == grid_sentinel_map.end()) {
					return GetClosestNonWaterLocation(loc, wander_dir);
				}
			}
	}

	return closest_grid;
}


Location State::GetClosestNonWaterLocation(Location& loc, int wander_dir) {
	Location location = loc;
	while (territory_map[location.row][location.col] == WATER) {
		location = getLocation(location, WANDER_DIRECTIONS[wander_dir][0], WANDER_DIRECTIONS[wander_dir][1]);
	}

	return location;
}


// Get a grid point that is invisible
Location State::GetClosetInvisibleGridLocation(Location& loc, int wander_dir) {
	int row_pos = (loc.row + grid_length / 2) / grid_length;
	int col_pos = (loc.col + grid_length / 2) / grid_length;

	int row_offset = grid_length * WANDER_DIRECTIONS[wander_dir][0];
	int col_offset = grid_length * WANDER_DIRECTIONS[wander_dir][1];

	Location closest_grid = getLocation(Location(0,0), row_pos * grid_length, col_pos * grid_length);
	if ( invisible_grid.find(closest_grid) != invisible_grid.end() || grid_sentinel_map.find(closest_grid) == grid_sentinel_map.end()
		|| grid_sentinel_map[closest_grid] == loc) {
		while (territory_map[closest_grid.row][closest_grid.col] == WATER) {
			closest_grid = getLocation(closest_grid, WANDER_DIRECTIONS[wander_dir][0], WANDER_DIRECTIONS[wander_dir][1]);
		}
		return closest_grid;
	}
	else {
		Location target = getLocation(closest_grid, row_offset, col_offset);
		while (territory_map[target.row][target.col] == WATER) {
			target = getLocation(target, WANDER_DIRECTIONS[wander_dir][0], WANDER_DIRECTIONS[wander_dir][1]);
		}

		return target;
	}
	
	/*Location target = getLocation(closest_grid, row_offset, col_offset);
	while (invisible_grid.find(target) == invisible_grid.end() && target != closest_grid) {
		target = getLocation(target, row_offset, col_offset);
	}
	return target;*/
}



// Get the closest grid point that has not been visited before
Location State::GetClosetUnvisitedGridLocation(Location& loc, int wander_dir) {
	int row_pos = (loc.row + WANDER_DIRECTIONS[wander_dir][0] * grid_length) / grid_length;
	int col_pos = (loc.col + WANDER_DIRECTIONS[wander_dir][1] * grid_length) / grid_length;

	for (int d = 0; d < NUM_WANDER_DIRECTIONS; d++) {
		int dir = (wander_dir + d) % NUM_WANDER_DIRECTIONS;
		int new_row_pos = row_pos + WANDER_DIRECTIONS[dir][0];
		int new_col_pos = col_pos + WANDER_DIRECTIONS[dir][1];
		Location grid_loc = getLocation(Location(0,0), new_row_pos * grid_length, new_col_pos * grid_length);
		if (unvisited_grid.find(grid_loc) != unvisited_grid.end()) {
			while (territory_map[grid_loc.row][grid_loc.col] == WATER) {
				grid_loc = getLocation(grid_loc, WANDER_DIRECTIONS[dir][0], WANDER_DIRECTIONS[dir][1]);
			}
			return grid_loc;
		}
	}

	return NULL_LOCATION;
}



//outputs move information to the engine
void State::makeMove(const Location &loc, int direction)
{
    cout << "o " << loc.row << " " << loc.col << " " << CDIRECTIONS[direction] << endl;

    Location nLoc = getLocation(loc, direction);
    grid[nLoc.row][nLoc.col].ant = grid[loc.row][loc.col].ant;
    grid[loc.row][loc.col].ant = -1;
};

//returns the euclidean distance between two locations with the edges wrapped
double State::distance(const Location &loc1, const Location &loc2)
{
    int d1 = abs(loc1.row-loc2.row),
        d2 = abs(loc1.col-loc2.col),
        dr = min(d1, rows-d1),
        dc = min(d2, cols-d2);

    return sqrt((double)(dr*dr + dc*dc));
};

//
int State::xyDistance(const Location & loc1, const Location &loc2) {
	int d1 = abs(loc1.row-loc2.row),
		d2 = abs(loc1.col-loc2.col),
		dr = min(d1, rows-d1),
		dc = min(d2, cols-d2);

	return (dr + dc);
}

//returns the euclidean distance between two locations with the edges wrapped
int State::distance2(const Location &loc1, const Location &loc2)
{
    int d1 = abs(loc1.row-loc2.row),
        d2 = abs(loc1.col-loc2.col),
        dr = min(d1, rows-d1),
        dc = min(d2, cols-d2);

    return (dr*dr + dc*dc);
};

//returns the new location from moving in a given direction with the edges wrapped
Location State::getLocation(const Location &loc, int direction)
{
    return Location( (loc.row + DIRECTIONS[direction][0] + rows) % rows,
                     (loc.col + DIRECTIONS[direction][1] + cols) % cols );
};


Location State::getLocation(const Location &loc, int row_offset, int col_offset) {
	return Location( (loc.row + row_offset + rows) % rows,
						(loc.col + col_offset + cols) % cols );
}

/*
    This function will update update the lastSeen value for any squares currently
    visible by one of your live ants.

    BE VERY CAREFUL IF YOU ARE GOING TO TRY AND MAKE THIS FUNCTION MORE EFFICIENT,
    THE OBVIOUS WAY OF TRYING TO IMPROVE IT BREAKS USING THE EUCLIDEAN METRIC, FOR
    A CORRECT MORE EFFICIENT IMPLEMENTATION, TAKE A LOOK AT THE GET_VISION FUNCTION
    IN ANTS.PY ON THE CONTESTS GITHUB PAGE.
*/
void State::updateVisionInformation()
{
    std::queue<Location> locQueue;
    Location sLoc, cLoc, nLoc;

    for(int a=0; a<(int) myAnts.size(); a++)
    {
        sLoc = myAnts[a];
        locQueue.push(sLoc);

        std::vector<std::vector<bool> > visited(rows, std::vector<bool>(cols, 0));
        grid[sLoc.row][sLoc.col].isVisible = 1;
        visited[sLoc.row][sLoc.col] = 1;

        while(!locQueue.empty())
        {
            cLoc = locQueue.front();
            locQueue.pop();

            for(int d=0; d<TDIRECTIONS; d++)
            {
                nLoc = getLocation(cLoc, d);

                if(!visited[nLoc.row][nLoc.col] && distance(sLoc, nLoc) <= viewradius)
                {
                    grid[nLoc.row][nLoc.col].isVisible = 1;
                    locQueue.push(nLoc);
                }
                visited[nLoc.row][nLoc.col] = 1;
            }
        }
    }
};

/*
    This is the output function for a state. It will add a char map
    representation of the state to the output stream passed to it.

    For example, you might call "cout << state << endl;"
*/
ostream& operator<<(ostream &os, const State &state)
{
    for(int row=0; row<state.rows; row++)
    {
        for(int col=0; col<state.cols; col++)
        {
            if(state.grid[row][col].isWater)
                os << '%';
            else if(state.grid[row][col].isFood)
                os << '*';
            else if(state.grid[row][col].isHill)
                os << (char)('A' + state.grid[row][col].hillPlayer);
            else if(state.grid[row][col].ant >= 0)
                os << (char)('a' + state.grid[row][col].ant);
            else if(state.grid[row][col].isVisible)
                os << '.';
            else
                os << '?';
        }
        os << endl;
    }

    return os;
};

//input function
istream& operator>>(istream &is, State &state)
{
    int row, col, player;
    string inputType, junk;

    //finds out which turn it is
    while(is >> inputType)
    {
        if(inputType == "end")
        {
            state.gameover = 1;
            break;
        }
        else if(inputType == "turn")
        {
            is >> state.turn;
            break;
        }
        else //unknown line
            getline(is, junk);
    }

    if(state.turn == 0)
    {
        //reads game parameters
        while(is >> inputType)
        {
            if(inputType == "loadtime")
                is >> state.loadtime;
            else if(inputType == "turntime")
                is >> state.turntime;
            else if(inputType == "rows")
                is >> state.rows;
            else if(inputType == "cols")
                is >> state.cols;
            else if(inputType == "turns")
                is >> state.turns;
            else if(inputType == "player_seed")
                is >> state.seed;
            else if(inputType == "viewradius2")
            {
				double tmp;
                is >> tmp;
				state.viewradius2 = (int)tmp;
                state.viewradius = sqrt((double)state.viewradius2);
            }
            else if(inputType == "attackradius2")
            {
				double tmp;
                is >> tmp;
				state.attackradius2 = tmp;
                state.attackradius = sqrt((double)state.attackradius2);
            }
            else if(inputType == "spawnradius2")
            {
				double tmp;
                is >> tmp;
				state.spawnradius2 = tmp;
                state.spawnradius = sqrt((double)state.spawnradius2);
            }
            else if(inputType == "ready") //end of parameter input
            {
                state.timer.start();
                break;
            }
            else    //unknown line
                getline(is, junk);
        }
    }
    else
    {
        //reads information about the current turn
        while(is >> inputType)
        {
            if(inputType == "w") //water square
            {
                is >> row >> col;
                state.grid[row][col].isWater = 1;
				state.water.push_back(Location(row, col));
            }
            else if(inputType == "f") //food square
            {
                is >> row >> col;
                state.grid[row][col].isFood = 1;
                state.food.push_back(Location(row, col));
            }
            else if(inputType == "a") //live ant square
            {
                is >> row >> col >> player;
                state.grid[row][col].ant = player;
                if(player == 0)
                    state.myAnts.push_back(Location(row, col));
                else
                    state.enemyAnts.push_back(Location(row, col));
            }
            else if(inputType == "d") //dead ant square
            {
                is >> row >> col >> player;
                state.grid[row][col].deadAnts.push_back(player);
				state.deadAnts[Location(row, col)] = player;
            }
            else if(inputType == "h")
            {
                is >> row >> col >> player;
                state.grid[row][col].isHill = 1;
                state.grid[row][col].hillPlayer = player;
                if(player == 0)
                    state.myHills.push_back(Location(row, col));
                else
                    state.enemyHills.push_back(Location(row, col));

            }
            else if(inputType == "players") //player information
                is >> state.noPlayers;
            else if(inputType == "scores") //score information
            {
                state.scores = vector<double>(state.noPlayers, 0.0);
                for(int p=0; p<state.noPlayers; p++)
                    is >> state.scores[p];
            }
            else if(inputType == "go") //end of turn input
            {
                if(state.gameover)
                    is.setstate(std::ios::failbit);
                else
                    state.timer.start();
                break;
            }
            else //unknown line
                getline(is, junk);
        }
    }

    return is;
};



//==================================== Extended Functions ==============================
vector<Location> State::GetMyAntsInArea(const Location& left_top, const Location& right_bottom) {
	vector<Location> my_ants;
	Location cur_loc = left_top;
	while (cur_loc != right_bottom) {
		if (IsMyAnt(cur_loc))
			my_ants.push_back(cur_loc);

		if (cur_loc.col == right_bottom.col) {
			cur_loc.col = left_top.col;
			cur_loc = getLocation(cur_loc, 1, 0);
		} else {
			cur_loc = getLocation(cur_loc, 0, 1);
		}
	}

	if (IsMyAnt(right_bottom))
		my_ants.push_back(right_bottom);

	return my_ants;
}


vector<Location> State::GetEnemyAntsInArea(const Location& left_top, const Location& right_bottom) {
	vector<Location> enemy_ants;
	Location cur_loc = left_top;
	while (cur_loc != right_bottom) {
		if (IsEnemyAnt(cur_loc))
			enemy_ants.push_back(cur_loc);

		if (cur_loc.col == right_bottom.col) {
			cur_loc.col = left_top.col;
			cur_loc = getLocation(cur_loc, 1, 0);
		} else {
			cur_loc = getLocation(cur_loc, 0, 1);
		}
	}

	if (IsEnemyAnt(right_bottom))
		enemy_ants.push_back(right_bottom);

	return enemy_ants;
}
