#ifndef BOT_H_
#define BOT_H_

#include "State.h"
#include "AntInfo.h"
#include "BattleResolver.h"

using namespace std;


struct TaskInfo {
	int			ant_id;
	int			task_type;
	Location	dest_loc;
	int			score;
	vector<int> path;

	friend bool operator==(const TaskInfo& l, const TaskInfo& r) {
		return (l.score == r.score);
	}

	friend bool operator<(const TaskInfo& l, const TaskInfo& r) {
		return (l.score < r.score);
	}
};


struct MoveStep {
	int			move_dir;
	TaskInfo	task;
};


struct PathInfo {
	Location		curr_location;
	int				dis_to_target;
	vector<int>		directions;
};



/*
    This struct represents your bot in the game of Ants
*/
struct Bot
{
    State state;

    Bot();

    void playGame();    //plays a single game of Ants

    void makeMoves();   //makes moves for a single turn
    void endTurn();     //indicates to the engine that it has made its moves

	set<Location>& GetMyFightersInTargetAttackArea(const Location& target);
	set<Location>& GetMyFightersInTargetDefendArea(const Location& target);
	set<Location>& GetMyFightersInTargetBackupArea(const Location& target);
	set<Location>& GetEnemiesInTargetAttackArea(const Location& target);
	set<Location>& GetEnemiesInTargetDefendArea(const Location& target);
	set<Location>& GetEnemiesInTargetBackupArea(const Location& target);


private:
	// Game strategy parameters
	// Maximum distance2 to the enemy hill when my ant should attack
	static const int MAX_PATH_STEPS = 100;
	static const int MAX_PATH_SEARCH_DISTANCE2 = 400;
	static const int MIN_PATH_SEARCH_DISTANCE2 = 36;
	static const int MAX_FOOD_PATH_SEARCH_DISTANCE2 = 300;
	static const int MAX_TASK_PATH_LENGTH = 100;
	static const int MAX_INTERCEPT_DISTANCE2 = 150;

	const static int ATTACK_DISTANCE2 = 25;
	const static int DEFENSE_DISTANCE2 = 200; //120; //16;
	const static int INNER_DEFENSE_DISTANCE2 = 16; //9;
	const static int OUTER_DEFENSE_DISTANCE2_MIN = 60;
	const static int OUTER_DEFENSE_DISTANCE2_MAX = 86;
	// Maximum number of my ants that simultaneously attack an enemy hill
	static const int MAX_GROUP_SIZE = 30;
	// The distance from which my ants should protect my hill
	static const int PROTECT_DISTANCE = 10;
	

	int num_rows, num_cols;
	int safe_turn_time_limit;
	int battle_time_limit;
	vector< vector<int> > my_visibility_map;
	vector< vector<int> > enemy_visibility_map;

	vector< vector<LocationOffset> > new_visibility_offsets;
	vector< vector<LocationOffset> > erased_visibility_offsets;
	set<LocationOffset> view_offsets;						// view area
	set<LocationOffset> attack_offsets;						// area that an ant can directly attack
	vector< set<LocationOffset> > defend_offsets;			// 1-offset from the attack area
	vector< set<LocationOffset> > backup_offsets;			// 2-offset from the attack area
	
	 
	// Food that have already been targets
	// Key: food location
	// Value: the ant id for this target
	map<Location, vector<int> > food_hunters;

	// Enemy hills that are currently my targets
	// Key: enemy hill location
	// Value: list of my ants that target this hill
	map<Location, vector<int> > hill_razers;

	// Enemies that I need to intercept
	// Key: enemy location
	// Value: list of my ants that intercepts this ant
	map<Location, vector<int> > enemy_intercepters;

	// All potential tasks for each of my ants
	// Key: my ant id
	// Value: a map with <Target_Location, TaskInfo> information
	map<int, map<Location, TaskInfo> > ant_tasks;

	map<int, vector<TaskInfo> > ant_task_vectors;

	// All my pending moves because of blocking
	// Key: the target move location 
	// Value: list of my ants that want to move to this location
	map<Location, vector<MoveStep> > pending_moves;
	set<Location> resolved_blocking_loc_set;


	// Key: enemy hill location
	// Value: the ant id of my sneaker for this hill
	// Note: one enemy hill should have at most one sneaker
	map<Location, vector<int> > sneakers;	

	// Key: location	
	// Value: IDs of the ants that can attack this location
	map<Location, set<Location> > my_attack_area_map;
	map<Location, set<Location> > my_defend_area_map;
	map<Location, set<Location> > my_backup_area_map;
	map<Location, set<Location> > my_view_area_map;
	map<Location, set<Location> > enemy_attack_area_map;
	map<Location, set<Location> > enemy_defend_area_map;
	map<Location, set<Location> > enemy_backup_area_map;
	map<Location, set<Location> > enemy_view_area_map;

	map<Location, int> start_ant_id_map;
	map<Location, int> curr_ant_id_map;
	map<Location, int> next_ant_id_map;
	map<int, AntInfo> ant_info_map;
	vector<int> num_wandering_ants;
	int curr_ant_id;
	map<Location, AntInfo> enemy_ant_info;

	void SetupGame();
	void UpdateMapInfo();
	void UpdateMyBattleFieldInfo();
	void UpdateEnemyBattleFieldInfo();
	void UpdateMyAntsStatus();
	void UpdateMyAntInfo(AntInfo& my_ant);
	void UpdateEnemyAntsStatus();
	void UpdateEnemyAntInfo(AntInfo& enemy_ant);

	int  num_idle_ants;
	int  num_remained_idle_ants;
	int	 num_interceptors;
	int  num_food_hunters;
	int  num_razers;
	int  num_map_explorers;
	int  num_wanderers;
	int	 max_num_interceptors;
	int  max_num_food_hunters;
	int  max_num_razers;
	int  max_num_wanderers;
	int  max_num_map_explorers;
	

	int	 GetMaxNumFireCover(bool is_hill_attacker);
	int	 GetMaxRazeGroupSize();
	int  GetMaxTotalRazerSize();
	int  GetTotalNumRazers();
	int	 GetMaxIncerceptorSize();
	//int  GetMaxNumInceptorAndRazers();
	int  GetMaxNumMapExplorer();
	int	 GetMaxNumWanderer();

	void		FightWithEnemies();
	void		ResolveBattle(Location& enemy_loc);
	set<Location> GetMyFightersInBattle(Location& target);
	set<Location> GetMyFightersInViewArea(Location& ant_loc);
	int			GetNumEnemiesInBattle(Location& target);
	int			GetNumMyFightersInBattle(const Location& target);
	

	void		ChooseBestPlan();
	void		GenerateAntTasks(set<Location>& my_ant_set);
	bool		IsTaskValid(TaskInfo& task);
	
	void		ResolvePendingMoves();
	void		ResolvePendingMove(const Location& blocking_loc);
	MoveStep&	GetBestBlockedMove(vector<MoveStep>& steps);


	TaskInfo	EvaluateHuntFoodTask(AntInfo& my_ant, const Location& food_loc);
	TaskInfo	EvaluateRazeHillTask(AntInfo& my_ant, const Location& hill_loc);
	TaskInfo	EvaluateInterceptEnemyTask(AntInfo& my_ant, const Location& enemy_loc);
	TaskInfo	EvaluateExploreMapTask(AntInfo& my_ant);
	TaskInfo	EvaluateWanderingTask(AntInfo& my_ant);
	Location	GetNextWanderingLocation(AntInfo& my_ant);
	void		ExecuteTask(TaskInfo& task);
	void		ExecuteHuntFoodTask(TaskInfo& task);
	void		ExecuteRazeEnemyHillTask(TaskInfo& task);
	void		ExecuteInterceptEnemyTask(TaskInfo& task);
	string		GetTaskString(int task_type);
	void		MoveToEnemyHill(AntInfo& my_ant, TaskInfo& task);

	void		DoMapExplorationOrWandering(AntInfo& my_ant);
	bool		DoMapExploration(AntInfo& my_ant);
	bool		IsMapExplorationBlocked(Location& my_loc, int wander_dir);
	bool		DoWandering(AntInfo& my_ant);
	bool		IsWanderBlocked(Location& my_loc, int wander_dir);
	int			GetWanderDirection(Location ant_loc);
	int			GetIncreasedVisibility(AntInfo& my_ant, int direction);
	bool		IsBlocked(Location loc);
	

	void		MoveToTarget(AntInfo& my_ant, TaskInfo& task);
	void		MoveToNextLocation(AntInfo& my_ant, int d, TaskInfo& task);
	void		SendMoveOrder(AntInfo& my_ant, int direction);
	void		SendMoveOrders(set<Location>& my_ants, vector<int>& directions);
	


	vector<int> FindShortestPath(const Location& src, const Location& dst, 
						int start_direction = 0, const int step_limit = 100, const bool is_safe = false);
	vector<int> FindPathAStar(const Location& src, const Location& dst, 
						int start_direction = 0, const int step_limit = 100, const bool is_safe = false);

	void PrintPath(const Location& src, const vector<int>& path);

	// Used for the comparison of location (IsCloserThan relationship)
	template <class ForwardIterator>
	ForwardIterator GetClosestLocation(Location loc, ForwardIterator first, ForwardIterator last );
};

#endif //BOT_H_
