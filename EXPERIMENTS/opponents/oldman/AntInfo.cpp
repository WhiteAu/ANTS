//------------------------- AntInfo.cpp ----------------------------

#include "AntInfo.h"

State AntInfo::null_state;
AntInfo::AntInfo() : state(AntInfo::null_state) {
	food_target = hill_target = NULL_LOCATION;
	has_moved = true;
}

// Constructor for my ants
AntInfo::AntInfo(State& s, int id): state(s), ant_id(id) {
	player_id = 0;
	wander_direction = id % 8;
	food_target = hill_target = NULL_LOCATION;
	weakest_defend_location = NULL_LOCATION;
	is_hill_defender = false;
	is_hill_attacker = false;
	is_sneaker = false;
	is_touch_down = false;
	has_moved = false;
	is_battle_resolved = false;
}

// Constructor for enemy ants
AntInfo::AntInfo(State& s, Location& loc, int pid) : state(s), location(loc), player_id(pid) {
	food_target = hill_target = NULL_LOCATION;
	weakest_defend_location = loc;
	is_hill_defender = is_hill_attacker = is_sneaker = false;
	is_touch_down = false;
	is_battle_resolved = false;
	has_moved = false;
}

void AntInfo::SetAntId(int id) {
	ant_id = id;
}

int AntInfo::GetAntId() {
	return ant_id;
}

int AntInfo::GetPlayerId() {
	return player_id;
}

int	AntInfo::GetWanderDirection() {
	return wander_direction;
}

void AntInfo::SetWanderDirection(int d) {
	wander_direction = d;
}

bool AntInfo::IsMapExplorer() {
	return (task.task_type == Task::TASK_EXPLORE_MAP);
}

void AntInfo::SetMapExplorer(bool b) {
	if (b)
		task.task_type = Task::TASK_EXPLORE_MAP;
	else
		task.task_type = Task::TASK_IDLE;
}

bool AntInfo::IsWandering() {
	return (task.task_type == Task::TASK_WANDERING);
}

void AntInfo::SetWandering(bool b) {
	if (b)
		task.task_type = Task::TASK_WANDERING;
	else
		task.task_type = Task::TASK_IDLE;
}

void AntInfo::SetCurrentLocation(const Location& loc) {
	location = loc;
}

Location AntInfo::GetCurrentLocation() {
	return location;
}

bool AntInfo::IsHillDefender() {
	return is_hill_defender;
}

void AntInfo::SetHillDefender(bool b) {
	is_hill_defender = b;
}

bool AntInfo::IsHillAttacker() {
	return is_hill_attacker;
}

void AntInfo::SetHillAttacker(bool b) {
	is_hill_attacker = b;
}

bool AntInfo::IsSneaker() {
	return is_sneaker;
}

void AntInfo::SetSneaker(bool b) {
	is_sneaker = b;
}

bool AntInfo::IsTouchDown() {
	return is_touch_down;
}


void AntInfo::SetTouchDown(bool b) {
	is_touch_down = b;
}


Location AntInfo::GetWeakestDefendLocation() {
	return weakest_defend_location;
}


void AntInfo::SetWeakestDefendLocation(Location loc) {
	weakest_defend_location = loc;
}

bool AntInfo::IsBattleResolved() {
	return is_battle_resolved;
}

void AntInfo::SetBattleResolved(bool b) {
	is_battle_resolved = b;
}


bool AntInfo::operator() (const Location& loc1, const Location& loc2) const {
	return (state.distance2(location, loc1) < state.distance2(location, loc2));
}

Location AntInfo::GetFoodTarget() {
	return food_target;
}

void AntInfo::SetFoodTarget(const Location& target) {
	food_target = target;
	task.task_type = Task::TASK_GET_FOOD;
}

Location AntInfo::GetHillTarget() {
	return hill_target;
}

void AntInfo::SetHillTarget(const Location& target) {
	hill_target = target;
	task.task_type = Task::TASK_RAZE_HILL;
}


//
bool AntInfo::IsTaskStillValid() {
	if (task.dst_loc == NULL_LOCATION)
		return false;

	switch (task.task_type) {
	case Task::TASK_GET_FOOD:
		if (state.grid[task.dst_loc.row][task.dst_loc.col].isFood ||
			!state.grid[task.dst_loc.row][task.dst_loc.col].isVisible)
			return true;
		else
			break;
	case Task::TASK_RAZE_HILL:
		if (state.grid[task.dst_loc.row][task.dst_loc.col].isHill ||
			!state.grid[task.dst_loc.row][task.dst_loc.col].isVisible)
			return true;
		else
			break;
	case Task::TASK_WANDERING:
		if (!state.grid[task.dst_loc.row][task.dst_loc.col].isWater ||
			!state.grid[task.dst_loc.row][task.dst_loc.col].isVisible)
			return true;
		else
			break;
	case Task::TASK_ATTACK_ENEMY:
	case Task::TASK_JOIN_GROUP:
	case Task::TASK_DEFEND_HILL:
		return true;
	default:
		break;
	}

	return false;
}

Task AntInfo::GetTask() {
	return task;
}

int AntInfo::GetTaskType() {
	return task.task_type;
}

void AntInfo::SetTaskType(int task_type) {
	task.task_type = task_type;
}


void AntInfo::SetNewTask(int type, const Location& dst, const vector<int>& path) {
	task.task_type = type;
	task.src_loc = location;
	task.dst_loc = dst;
	task.path_to_dst = path;
	task.current_path_step = 0;
	task.task_start_turn = state.turn;
}


int AntInfo::GetDistanceToDestination() {
	return (task.path_to_dst.size() - task.current_path_step);
}

int AntInfo::GetNextMove() {
	if (has_moved)
		return -1;

	if (task.dst_loc == NULL_LOCATION) {
		return -1;
	}
	else if (task.current_path_step == task.path_to_dst.size()) {
		FinishTask();
		return -1;
	}
	else {
		return task.path_to_dst[task.current_path_step];
	}
}

Location AntInfo::GetNextLocation() {
	int d = GetNextMove();
	if (d != -1) {
		return state.getLocation(location, d);
	}
	else 
		return location;
}

/////
Location AntInfo::GetLocationInPath(int step_offset) {
	int target_step = task.current_path_step + step_offset;
	if (target_step < task.path_to_dst.size()) {
		Location loc = location;
		for (int i = task.current_path_step; i <= target_step; i++) {
			loc = state.getLocation(loc, task.path_to_dst[i]);
		}
		return loc;
	}
	else {
		return NULL_LOCATION;
	} 
}


///
void AntInfo::ChangePath(int until_step_offset, vector<int>& new_path) {
	vector<int> new_path_to_dst = new_path;
	new_path.insert(new_path_to_dst.end(), task.path_to_dst[task.current_path_step + until_step_offset + 1], 
					task.path_to_dst[task.path_to_dst.size() - 1]);

	task.src_loc = location;
	task.path_to_dst = new_path_to_dst;
	task.current_path_step = 0;
	task.task_start_turn = state.turn;
}

//
vector<int> AntInfo::GetRemainedPath() {
	vector<int> path;
	for (int i = task.current_path_step; i < task.path_to_dst.size(); i++) {
		path.push_back(task.path_to_dst[i]);
	}

	return path;
}

///
static int NUM_PATH_STEPS_CHECKING_AHEAD = 12;
bool AntInfo::IsPathStillValid() {
	int step = task.current_path_step;
	Location loc = location;
	for (int i = 0; i < NUM_PATH_STEPS_CHECKING_AHEAD ; i++) {
		if ((step + i) < task.path_to_dst.size()) {
			loc = state.getLocation(loc, task.path_to_dst[step + i]);
			if (state.IsWater(loc)) {
				state.bug << "Path becomes invalid!" << endl;
				return false;
			}
		}
		else
			break;
	}
	
	return true;
}


void AntInfo::FinishTask() {
	task.task_type = Task::TASK_IDLE;
	task.src_loc = task.dst_loc = NULL_LOCATION;
	task.path_to_dst.clear();
	task.current_path_step = 0;

	food_target = hill_target = NULL_LOCATION;
	is_hill_attacker = false;
	is_hill_defender = false;
	is_sneaker = false;
	is_touch_down = false;
}


void AntInfo::MakeMove(int direction) {
	state.makeMove(location, direction);
	task.current_path_step++;
	has_moved = true;
	location = state.getLocation(location, direction);
}

void AntInfo::FinishTurnWithoutMove() {
	has_moved = true;
}

void AntInfo::SetMoved(bool moved) {
	has_moved = moved;
}

bool AntInfo::HasMoved() {
	return has_moved;
}

Location AntInfo::GetTaskSrcLocation() {
	return task.src_loc;
}

Location AntInfo::GetTaskDstLocation() {
	return task.dst_loc;
}
