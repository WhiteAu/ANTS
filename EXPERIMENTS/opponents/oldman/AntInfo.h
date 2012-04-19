//------------------------------- AntInfo.h --------------------------------

#ifndef ANTINFO_H
#define ANTINFO_H

#include <functional>
#include "State.h"

using namespace std;

struct Task{
	// Task types
	static const int TASK_IDLE = 0;				// No task has currently been assigned
	static const int TASK_GET_FOOD = 1;			// Get food to spawn a new ant
	static const int TASK_EXPLORE_MAP = 2;	// Join a group of other ants of my own
	static const int TASK_WANDERING = 3;		// Just wandering
	static const int TASK_RAZE_HILL = 4;		// Raze enemy hill
	static const int TASK_DEFEND_HILL = 5;		// Defend my own hill
	static const int TASK_ATTACK_ENEMY = 6;		// Go attack other enemy ants
	static const int TASK_JOIN_GROUP = 7;		// Join a group of other ants of my own

	int task_type;
	int task_start_turn;
	int current_path_step;
	Location src_loc, dst_loc;
	vector<int> path_to_dst;

	Task() {
		src_loc = dst_loc = NULL_LOCATION;
		task_type = TASK_IDLE;
	}
};


class AntInfo {
public:
	AntInfo();
	AntInfo(State& s, int id);
	AntInfo(State& s, Location& loc, int pid);

	int			GetNextMove();
	Location	GetNextLocation();
	Location	GetLocationInPath(int step_offset);
	void		MakeMove(int direction);
	void		FinishTurnWithoutMove();
	void		SetMoved(bool moved);
	bool		HasMoved();
	bool		IsHillDefender();
	bool		IsTouchDown();
	bool		IsHillAttacker();
	bool		IsSneaker();
	bool		IsWandering();
	bool		IsMapExplorer();
	void		SetHillDefender(bool b);
	void		SetHillAttacker(bool b);
	void		SetSneaker(bool b);
	void		SetTouchDown(bool b);
	void		SetWandering(bool b);
	void		SetMapExplorer(bool b);

	bool		IsBattleResolved();
	void		SetBattleResolved(bool b);
	Location	GetWeakestDefendLocation();
	void		SetWeakestDefendLocation(Location loc);

	void		SetAntId(int id);
	int			GetAntId();
	int			GetPlayerId();
	int			GetWanderDirection();
	void		SetWanderDirection(int d);

	void		SetCurrentLocation(const Location& loc);
	Location	GetCurrentLocation();
	Location	GetTaskSrcLocation();
	Location	GetTaskDstLocation();
	Task		GetTask();
	void		FinishTask();

	Location	GetFoodTarget();
	void		SetFoodTarget(const Location& target);
	Location	GetHillTarget();
	void		SetHillTarget(const Location& target);

	
	//void		UpdateStatus();
	bool		IsTaskStillValid();
	bool		IsPathStillValid();
	int			GetTaskType();
	void		SetTaskType(int task_type);
	void		SetNewTask(int type, const Location& dst, const vector<int>& path);
	int			GetDistanceToDestination();
	void		ChangePath(int until_step_offset, vector<int>& new_path);
	vector<int> GetRemainedPath();

	bool operator() (const Location& loc1, const Location& loc2) const;
	static State null_state;

private:
	
	State& state;
	int player_id;
	int ant_id;
	int	wander_direction;
	Location location;
	bool is_hill_defender;
	bool is_hill_attacker;
	bool is_sneaker;
	bool is_touch_down;
	bool has_moved;

	// for enemy ants
	Location  weakest_defend_location;
	bool is_battle_resolved;
	
	Location food_target;
	Location hill_target;

	Task task;
};

#endif 