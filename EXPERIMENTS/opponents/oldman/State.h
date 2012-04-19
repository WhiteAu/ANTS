#ifndef STATE_H_
#define STATE_H_

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <queue>
#include <list>
#include <stack>
#include <map>
#include <set>
#include <stdint.h>
#include <algorithm>

#include "Timer.h"
#include "Bug.h"
#include "Square.h"
#include "Location.h"

using namespace std;

#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))

/*
    constants
*/
const int TDIRECTIONS = 4;
const char CDIRECTIONS[4] = {'N', 'E', 'S', 'W'};
const int DIRECTIONS[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };      //{N, E, S, W}


const int FIND_PATH_DIRECTIONS[8][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}, {-1, 0}, {0, 1}, {1, 0}, {0, -1}};

const int WANDER_DIRECTIONS[8][2] = { {-1, -1}, {-1, 0}, {-1, 1}, {0, 1},
										{1, 1}, {1, 0}, {1, -1}, {0, -1} };
const int WANDER_STEPS[8][2] = {{0, 3}, {0, 0}, {0, 1}, {1, 1}, 
								{1, 2}, {2, 2}, {2, 3}, {3, 3} };
const int NUM_WANDER_DIRECTIONS = 8;

static const int UNKNOWN = -1;
static const int MY_ANT = 0;
static const int WATER = 10000;
static const int MY_HILL = 10001;
static const int ENEMY_HILL = 10002;
static const int MOVE_NOWAY = -1;
static const int MOVE_STAY = -2;

static const int MAX_RING_DISTANCE = 8;
static const int WATER_BLOCKING_PENALTY = 100;
static const int FOOD_BONUS_UNIT = 1060000;
static const int ENEMY_HILL_BONUS_UNIT = 500; //60000;
/*
    struct to store current state information
*/
struct State
{
    /*
        Variables
    */
    int rows, cols,
        turn, turns,
        noPlayers;
    double attackradius, spawnradius, viewradius;
	int attackradius2, spawnradius2, viewradius2;
    double loadtime, turntime;
    std::vector<double> scores;
    bool gameover;
    int64_t seed;

    std::vector<std::vector<Square> > grid;
    std::vector<Location> myAnts, enemyAnts, myHills, enemyHills, food, water;
	map<Location, int> deadAnts;

    Timer timer;
    Bug bug;


	// My own data structures for accumulative state information
	vector< vector<int> > territory_map;
	set<Location> my_original_hills;
	set<Location> my_remained_hills;
	set<Location> detected_enemy_hills;
	set<Location> detected_food;

	set<Location> visited_map;
	set<Location> unvisited_map;
	set<Location> all_grid_locs;
	set<Location> visited_grid;
	set<Location> unvisited_grid;
	set<Location> invisible_grid;
	map<Location, Location> grid_sentinel_map;
	int total_num_grids;
	int	grid_length;

	vector< vector<int> > water_blocking_penalties;
	vector< vector<int> > tile_bonus_values;
	vector< vector<LocationOffset> > distance_rings;


    /*
        Functions
    */
    State();
    ~State();

    void setup();
    void reset();
    void makeMove(const Location &loc, int direction);

	void UpdateState();
	void SetSentinel(Location grid_loc, int wander_dir);
	Location GetClosestInvisibleGridLocation(Location& loc, int wander_dir);
	Location GetClosestUnvisitedGridLocation(Location& loc, int wander_dir);
	Location GetClosestNonWaterLocation(Location& loc, int wander_dir);

	Location GetClosetInvisibleGridLocation(Location& loc, int wander_dir);
	Location GetClosetUnvisitedGridLocation(Location& loc, int wander_dir);

    double distance(const Location &loc1, const Location &loc2);
	int xyDistance(const Location & loc1, const Location &loc2);
	int distance2(const Location &loc1, const Location &loc2);
    Location getLocation(const Location &startLoc, int direction);
	Location getLocation(const Location &startLoc, int row_offset, int col_offset);
	
	

    void updateVisionInformation();

	// Extended functions
	inline bool IsMyHill(const Location& loc);
	inline bool IsEnemyHill(const Location& loc);
	inline bool IsWater(const Location& loc);
	inline bool IsFood(const Location& loc);
	inline bool IsMyAnt(const Location& loc);
	inline bool IsEnemyAnt(const Location& loc);
	inline bool IsPlayerAnt(const Location& loc, int player_id);
	inline bool IsPlayerEnemyAnt(const Location& loc, int player_id);
	inline bool IsVisible(const Location& loc);
	inline bool IsBlocked(const Location& loc);

	vector<Location> GetMyAntsInArea(const Location& left_top, const Location& right_bottom);
	vector<Location> GetEnemyAntsInArea(const Location& left_top, const Location& right_bottom);
};

std::ostream& operator<<(std::ostream &os, const State &state);
std::istream& operator>>(std::istream &is, State &state);


//==================================== Extended Inline Functions ==============================
inline bool State::IsMyHill(const Location& loc) {
	if (loc == NULL_LOCATION)
		return false;
	else
		return (grid[loc.row][loc.col].isHill && 
				grid[loc.row][loc.col].hillPlayer == 0);
}

inline bool State::IsEnemyHill(const Location& loc) {
	if (loc == NULL_LOCATION)
		return false;
	else
		return (grid[loc.row][loc.col].isHill && 
				grid[loc.row][loc.col].hillPlayer > 0);
}

inline bool State::IsWater(const Location& loc) {
	if (loc == NULL_LOCATION)
		return false;
	else
		return (territory_map[loc.row][loc.col] == WATER);
}

inline bool State::IsFood(const Location& loc) {
	if (loc == NULL_LOCATION)
		return false;
	else
		return grid[loc.row][loc.col].isFood;
}

inline bool State::IsMyAnt(const Location& loc) {
	if (loc == NULL_LOCATION)
		return false;
	else
		return grid[loc.row][loc.col].ant == 0;
}

inline bool State::IsEnemyAnt(const Location& loc) {
	if (loc == NULL_LOCATION)
		return false;
	else
		return grid[loc.row][loc.col].ant > 0;
}

inline bool State::IsPlayerAnt(const Location& loc, int player_id) {
	if (loc == NULL_LOCATION)
		return false;
	else 
		return grid[loc.row][loc.col].ant == player_id;
}

inline bool State::IsPlayerEnemyAnt(const Location& loc, int player_id) {
	if (loc == NULL_LOCATION)
		return false;
	else 
		return (grid[loc.row][loc.col].ant >= 0 &&
				grid[loc.row][loc.col].ant != player_id);
}

inline bool State::IsVisible(const Location& loc) {
	if (loc == NULL_LOCATION)
		return false;

	return grid[loc.row][loc.col].isVisible;
}

inline bool State::IsBlocked(const Location& loc) {
	if (loc == NULL_LOCATION)
		return false;

	return (territory_map[loc.row][loc.col] == WATER || grid[loc.row][loc.col].isFood ||
			grid[loc.row][loc.col].ant >= 0);
}

#endif //STATE_H_
