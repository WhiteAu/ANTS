#include "Bot.h"

using namespace std;

//constructor
Bot::Bot()
{
};

//plays a single game of Ants.
void Bot::playGame()
{
    //reads the game parameters and sets up
    cin >> state;
    state.setup();
	SetupGame();
    endTurn();

    //continues making moves while the game is not over
    while(cin >> state)
    {
        state.updateVisionInformation();
		state.UpdateState();
		UpdateMapInfo();
        makeMoves();
        endTurn();
    }
};

//makes the bots moves for the turn
void Bot::makeMoves()
{
    state.bug << "turn " << state.turn << ":" << endl;
    state.bug << state << endl;

	state.bug << "====================================== Update Ant Status ======================================" << endl;
	UpdateMyAntsStatus();
	UpdateEnemyAntsStatus();
	state.bug << "Status Analysis Time: " << state.timer.getTime() << "ms" << endl << endl;

	// Do fightings for all my ants that encounter with enemies
	FightWithEnemies();

	// Then choose the best move plans for all my other ants
	ChooseBestPlan();

	// Add remaining unmoved ant to ant_id_map 
	for (int i = 0; i < state.myAnts.size(); i++) {
		AntInfo& my_ant = ant_info_map[start_ant_id_map[state.myAnts[i]]];
		if (!my_ant.HasMoved()) {
			my_ant.FinishTurnWithoutMove();
			next_ant_id_map[my_ant.GetCurrentLocation()] = my_ant.GetAntId();
		}
	}

	// Update my ant map
	start_ant_id_map = curr_ant_id_map;
	curr_ant_id_map = next_ant_id_map;

    state.bug << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
};


// FIGHT!!
void Bot::FightWithEnemies() {
	state.bug << "===================================== Fight with Enemies =====================================" << endl;
	vector<Location>& enemy_locs = state.enemyAnts;
	int num_enemies  = enemy_locs.size();
	for (int e = 0; e < num_enemies; e++) {
		if (enemy_ant_info[enemy_locs[e]].IsBattleResolved())
			continue;

		int num_my_fighters = my_attack_area_map[enemy_locs[e]].size() + 
						      my_defend_area_map[enemy_locs[e]].size() +
							  my_backup_area_map[enemy_locs[e]].size();
		if (num_my_fighters > 0) {
			state.bug << "#### Start resolving battle for location " << enemy_locs[e] << " ####" << endl;
			ResolveBattle(enemy_locs[e]);
			state.bug << "Finish resolving battle for location " << enemy_locs[e] << endl;

			double curr_time = state.timer.getTime();
			state.bug << "Current Time: " << curr_time << " ms" << endl;
			if (state.timer.getTime() > battle_time_limit) {
				state.bug << "Battle time exceeded. Return immediately." << endl;
				return;
			}
		}
	}
}


//
void Bot::ResolveBattle(Location& enemy_loc) {
	set<Location>	my_ant_locs;
	set<Location>	enemy_ant_locs;
	enemy_ant_locs.insert(enemy_loc);
	vector<AntInfo*> my_ants;
	vector<AntInfo*> enemy_ants;
	enemy_ants.push_back(&enemy_ant_info[enemy_loc]);

	set<Location>::iterator it;
	set<Location>::iterator mit;
	set<Location>::iterator eit;

	// Searching for all the ants involved in the battle field
	set<Location> new_enemies;
	new_enemies.insert(enemy_loc);
	bool finish_searching = false;
	while (!finish_searching) {
		set<Location> my_new_ants;
		for (it = new_enemies.begin(); it != new_enemies.end(); it++) {
			// Get locations for all my ants and enemy ants that are involved in the battle			
			set<Location>& my_defenders = GetMyFightersInTargetDefendArea(*it);
			set<Location>& my_backups = GetMyFightersInTargetBackupArea(*it);
	
			for (mit = my_defenders.begin(); mit != my_defenders.end(); mit++) {
				// my ant may have already moved
				AntInfo& my_ant = ant_info_map[start_ant_id_map[*mit]];
				Location curr_loc = my_ant.GetCurrentLocation();
				if (my_ant_locs.find(curr_loc) == my_ant_locs.end()) {
					my_new_ants.insert(*mit);
					my_ant_locs.insert(curr_loc);
					my_ants.push_back(&my_ant);
				}
			}

			for (mit = my_backups.begin(); mit != my_backups.end(); mit++) {
				AntInfo& my_ant = ant_info_map[start_ant_id_map[*mit]];
				Location curr_loc = my_ant.GetCurrentLocation();
				if (my_ant_locs.find(curr_loc) == my_ant_locs.end()) {
					my_new_ants.insert(*mit);
					my_ant_locs.insert(curr_loc);
					my_ants.push_back(&my_ant);
				}
			}
		}

		// Searching for enemy ants
		new_enemies.clear();
		for (it = my_new_ants.begin(); it != my_new_ants.end(); it++) {
			set<Location>& enemy_defenders = GetEnemiesInTargetDefendArea(*it);
			set<Location>& enemy_backups = GetEnemiesInTargetBackupArea(*it);

			for (eit = enemy_defenders.begin(); eit != enemy_defenders.end(); eit++) {
				if (enemy_ant_locs.find(*eit) == enemy_ant_locs.end()) {
					new_enemies.insert(*eit);
					enemy_ant_locs.insert(*eit);
					enemy_ants.push_back(&enemy_ant_info[*eit]);
				}
			}

			for (eit = enemy_backups.begin(); eit != enemy_backups.end(); eit++) {
				if (enemy_ant_locs.find(*eit) == enemy_ant_locs.end()) {
					new_enemies.insert(*eit);
					enemy_ant_locs.insert(*eit);
					enemy_ants.push_back(&enemy_ant_info[*eit]);
				}
			}
		}

		if (new_enemies.size() == 0)
			finish_searching = true;
	}

	// Try to avoid enemies if I'm a map explorer 
	/*if (enemy_ants.size() == 1) {
		Location enemy_loc = enemy_ants[0]->GetCurrentLocation();
		vector<AntInfo*> new_fighter_group;
		for (int i = 0; i < my_ants.size(); i++) {
			if ( (my_ants[i]->IsMapExplorer() || my_ants[i]->IsWandering() ) && !my_ants[i]->IsHillDefender()) {
				state.bug << "Ant " << my_ants[i]->GetCurrentLocation() << " is a map explorer." << endl;
				Location my_loc = my_ants[i]->GetCurrentLocation();
				Location dodge_destination = state.GetClosestInvisibleGridLocation(my_ants[i]->GetCurrentLocation(), 
												my_ants[i]->GetWanderDirection());
				
				state.bug << "Dodge destination: " << dodge_destination << endl;
				vector<int> path = FindShortestPath(my_loc,  dodge_destination, 
													my_ants[i]->GetWanderDirection() >> 1, MAX_PATH_STEPS, true);
				if (path.size() > 0) {
					TaskInfo task;
					task.ant_id = my_ants[i]->GetAntId();
					task.task_type = my_ants[i]->GetTaskType();
					task.dest_loc = dodge_destination;
					task.score = 1000000;

					my_ants[i]->FinishTask();
					my_ants[i]->SetNewTask(my_ants[i]->GetTaskType(), dodge_destination, path);
					MoveToNextLocation(*my_ants[i], my_ants[i]->GetNextMove(), task);
				}
				else {
					new_fighter_group.push_back(my_ants[i]);
				}
			}
			else {
				new_fighter_group.push_back(my_ants[i]);
			}
		}


		if (new_fighter_group.size() == 0) {
			return;
		}
		else {
			my_ants = new_fighter_group;
		}
	}*/

	// Prepare information for the battle resolver
	BattleResolver resolver(state, my_ants, enemy_ants, enemy_loc);
	resolver.ResolveBattle();
	MovePlan& plan = resolver.GetBestMovePlan();
	for (int i = 0; i < my_ants.size(); i++) {
		if (!my_ants[i]->HasMoved()) {
			AntInfo& my_ant = ant_info_map[start_ant_id_map[my_ants[i]->GetCurrentLocation()]];
			my_ant.FinishTask();
			SendMoveOrder(my_ant, plan.directions[i]);
		}
	}

	for (it = enemy_ant_locs.begin(); it != enemy_ant_locs.end(); it++) {
		enemy_ant_info[*it].SetBattleResolved(true);
	}
}



// Get all my ants involved in the battle location
set<Location> Bot::GetMyFightersInBattle(Location& target) {
	set<Location> fighters;
	fighters.insert(my_attack_area_map[target].begin(), 
		my_attack_area_map[target].end());
	fighters.insert(my_defend_area_map[target].begin(), 
		my_defend_area_map[target].end());
	fighters.insert(my_backup_area_map[target].begin(), 
		my_backup_area_map[target].end());

	return fighters;
}

//
int Bot::GetNumEnemiesInBattle(Location& target) {
	return enemy_attack_area_map[target].size() +
		enemy_defend_area_map[target].size() +
		enemy_backup_area_map[target].size();
}

//
int Bot::GetNumMyFightersInBattle(const Location& target) {
	return my_attack_area_map[target].size() +
			my_defend_area_map[target].size() +
			my_backup_area_map[target].size();
}


set<Location> Bot::GetMyFightersInViewArea(Location& ant_loc) {
	set<Location> fellows_in_view_area;
	set<LocationOffset>::iterator it;
	for (it = view_offsets.begin(); it != view_offsets.end(); it++) {
		Location loc = state.getLocation(ant_loc, it->row_offset, it->col_offset);
		if (state.IsMyAnt(loc)) {
			fellows_in_view_area.insert(loc);
		}
	}
	return fellows_in_view_area;
}


// Get my ants in the target location's attack area
set<Location>& Bot::GetMyFightersInTargetAttackArea(const Location& target) {
	return my_attack_area_map[target];
}


// Get my ants in the target location's defend line
set<Location>& Bot::GetMyFightersInTargetDefendArea(const Location& target) {
	return my_defend_area_map[target];
}

// Get my ants in the target location's backup line
set<Location>& Bot::GetMyFightersInTargetBackupArea(const Location& target) {
	return my_backup_area_map[target];
}


set<Location>& Bot::GetEnemiesInTargetAttackArea(const Location& target) {
	return enemy_attack_area_map[target];
}

set<Location>& Bot::GetEnemiesInTargetDefendArea(const Location& target) {
	return enemy_defend_area_map[target];
}

set<Location>& Bot::GetEnemiesInTargetBackupArea(const Location& target) {
	return enemy_backup_area_map[target];
}




//////////////////////////////////
void Bot::ChooseBestPlan() {
	state.bug << "=============================== Choose Best Moves for My Ants ================================" << endl;
	vector<Location>& my_ants = state.myAnts;
	set<Location> my_left_ant_set;
	for (int i = 0; i < my_ants.size(); i++) {
		AntInfo& my_ant = ant_info_map[start_ant_id_map[my_ants[i]]];
		if (my_ant.HasMoved())
			continue;

		if (state.food.size() > 25 &&  my_ant.GetTaskType() == Task::TASK_GET_FOOD 
				&& state.detected_food.find(my_ant.GetFoodTarget()) != state.detected_food.end() ) {
			TaskInfo task;
			task.ant_id = my_ant.GetAntId();
			task.dest_loc = my_ant.GetFoodTarget();
			task.task_type = Task::TASK_GET_FOOD;
			task.score = 0;
			task.path = vector<int>(my_ant.GetDistanceToDestination(), 0);
			ExecuteTask(task);

			food_hunters[my_ant.GetFoodTarget()].push_back(my_ant.GetAntId());
			ant_tasks[task.ant_id][task.dest_loc] = task;
			continue;
		}
		
		my_left_ant_set.insert(my_ants[i]);
	}


	// Generate all valid tasks for my ants that have not moved
	GenerateAntTasks(my_left_ant_set);
	state.bug << "All ant tasks generated. Current time: " << state.timer.getTime() << " ms" << endl;

	set<Location>::iterator ant_it;
	num_idle_ants = my_left_ant_set.size();
	num_remained_idle_ants = num_idle_ants;
	max_num_map_explorers = GetMaxNumMapExplorer();
	max_num_interceptors = GetMaxIncerceptorSize();
	max_num_razers = GetMaxTotalRazerSize();
	//max_num_food_hunters;
	//max_num_map_explorers;
	//max_num_wanderers;

	
	set<Location>::iterator best_task_ant_it;
	vector<TaskInfo>::iterator best_task_it;
	vector<TaskInfo>::iterator task_it;
	for (int i = 0; i < num_idle_ants; i++) {
		state.bug << "************** Best Move " << (i+1) << " **************" << endl;
		int best_score = -99999999;
		for (ant_it = my_left_ant_set.begin(); ant_it != my_left_ant_set.end(); ant_it++) {
			AntInfo& my_ant = ant_info_map[start_ant_id_map[*ant_it]];
			int ant_id = my_ant.GetAntId();

			vector<TaskInfo>& tasks = ant_task_vectors[ant_id];
			for (task_it = tasks.begin(); task_it != tasks.end(); task_it++) {
				TaskInfo& task = *task_it;
				if (!IsTaskValid(task))
					continue;
				else {
					if (task.score > best_score) {
						best_score = task.score;
						best_task_it = task_it;
						best_task_ant_it = ant_it;
					}
					break;
				}
			}
		}

		if (best_score > 0) {
			state.bug << "Best task goes to Ant #" << best_task_it->ant_id << 
				" at " << ant_info_map[best_task_it->ant_id].GetCurrentLocation() << endl;
			state.bug << "Target Location: " << best_task_it->dest_loc 	<< "     Task Type: " << 
				GetTaskString(best_task_it->task_type) << "     Score: " << best_task_it->score << endl;

			ExecuteTask(*best_task_it);
			my_left_ant_set.erase(best_task_ant_it);
		}
		else {
			break;
		}

		if (state.timer.getTime() > safe_turn_time_limit) {
			state.bug << "***** Time exceeded. Stop immediately. *****" << endl;
			return;
		}
	}

	// Do map exploration or wandering
	state.bug << "============================= Do Map Exploration or Wandering ==============================" << endl;
	for (ant_it = my_left_ant_set.begin(); ant_it != my_left_ant_set.end(); ant_it++) {
		AntInfo& my_ant = ant_info_map[start_ant_id_map[*ant_it]];
		if (my_ant.IsWandering()) {
			state.SetSentinel(my_ant.GetTaskDstLocation(), ant_info_map[start_ant_id_map[*ant_it]].GetWanderDirection());
		}
		
	}

	for (ant_it = my_left_ant_set.begin(); ant_it != my_left_ant_set.end(); ant_it++) {
		DoMapExplorationOrWandering(ant_info_map[start_ant_id_map[*ant_it]]);

		if (state.timer.getTime() > safe_turn_time_limit) {
			state.bug << "***** Time exceeded. Stop immediately. *****" << endl;
			return;
		}
	}


	// Resolve all blocked moves
	ResolvePendingMoves();

	state.bug << "============================= Task Schedule Finished ==============================" << endl;
	state.bug << "Total # My Ants: " << state.myAnts.size() << endl;
	state.bug << "Total # Idle Ants: " << num_idle_ants << endl;
	state.bug << "Total # Food Hunters: " << num_food_hunters << endl;
	state.bug << "Total # Interceptors: " << num_interceptors << endl;
	state.bug << "Total # Hill Razers: " << num_razers << endl;
	state.bug << "Total # Map Explorers: " << num_map_explorers << endl;
	state.bug << "Total # Wanderers: " << num_wanderers << endl;
}


// Generate all valid tasks for my ants
void Bot::GenerateAntTasks(set<Location>& my_ant_set) {
	vector<Location>& my_ants = state.myAnts;
	vector<Location>& enemies = state.enemyAnts;

	set<Location>::iterator ant_it;
	set<Location>::iterator food_it;
	set<Location>::iterator enemy_it;
	set<Location>::iterator hill_it;

	TaskInfo task_info;
	for (ant_it = my_ant_set.begin(); ant_it != my_ant_set.end(); ant_it++) {
		AntInfo& my_ant = ant_info_map[start_ant_id_map[*ant_it]];
		Location my_loc = my_ant.GetCurrentLocation();
		int ant_id = my_ant.GetAntId();

		// Evaluating all the food hunt tasks
		for (food_it = state.detected_food.begin(); food_it != state.detected_food.end(); food_it++) {
			if (state.distance2(my_loc, *food_it) > MAX_FOOD_PATH_SEARCH_DISTANCE2)
				continue;

			task_info = EvaluateHuntFoodTask(my_ant, *food_it);
			if (task_info.score > 0) {
				ant_tasks[ant_id][*food_it] = task_info;
				ant_task_vectors[ant_id].push_back(task_info);
			}
		}

		// Evaluate razing enemy hill tasks 
		for (hill_it = state.detected_enemy_hills.begin(); hill_it != state.detected_enemy_hills.end(); hill_it++) {
			task_info = EvaluateRazeHillTask(my_ant, *hill_it);
			if (task_info.score > 0) {
				ant_tasks[ant_id][*hill_it] = task_info;
				ant_task_vectors[ant_id].push_back(task_info);
			}
		}


		// Evaluate intercepting enemy hill tasks 
		for (int e = 0; e < enemies.size(); e++) {
			task_info = EvaluateInterceptEnemyTask(my_ant, enemies[e]);
			if (task_info.score > 0) {
				ant_tasks[ant_id][enemies[e]] = task_info;
				ant_task_vectors[ant_id].push_back(task_info);
			}
		}

		make_heap(ant_task_vectors[ant_id].begin(), ant_task_vectors[ant_id].end());
	}
}


// Resolve all pending moves due to blocking
void Bot::ResolvePendingMoves() {
	state.bug << "=============================== Revolve Pending Moves ================================" << endl;
	map<Location, vector<MoveStep> >::iterator it;
	resolved_blocking_loc_set.clear();
	for (it = pending_moves.begin(); it != pending_moves.end(); it++) {
		if (resolved_blocking_loc_set.find(it->first) == resolved_blocking_loc_set.end())
			ResolvePendingMove(it->first);
	}
}

// Resolve the pending moves to a specific location
void Bot::ResolvePendingMove(const Location& blocking_loc) { 
	state.bug << "Resolving pending moves for location " << blocking_loc << ":";
	resolved_blocking_loc_set.insert(blocking_loc);

	MoveStep& best_step = GetBestBlockedMove(pending_moves[blocking_loc]);
	// Check if the blocking ant has moved to somewhere else
	if (!IsBlocked(blocking_loc)) {
		SendMoveOrder(ant_info_map[best_step.task.ant_id], best_step.move_dir);
		return;
	}
	
	
	if (curr_ant_id_map.find(blocking_loc) != curr_ant_id_map.end()) {
		AntInfo& blocking_ant = ant_info_map[curr_ant_id_map[blocking_loc]];

		// the ant that is blocking me has already moved to the location, nothing to do unless for the hill razers
		if (blocking_ant.HasMoved()) {
			if (best_step.task.task_type == Task::TASK_RAZE_HILL && 
				state.detected_enemy_hills.find(best_step.task.dest_loc) != state.detected_enemy_hills.end()) {
					AntInfo& my_ant = ant_info_map[best_step.task.ant_id];
					my_ant.FinishTask();
					my_ant.SetHillTarget(best_step.task.dest_loc);
					my_ant.SetHillAttacker(true);
					MoveToEnemyHill(my_ant, best_step.task);
			}
			return;
		}
		else {  // the ant is also blocked by someone else
			AntInfo& best_ant = ant_info_map[best_step.task.ant_id];
			Location best_ant_curr_loc = best_ant.GetCurrentLocation();

			// Check if the ant that is blocking me is also blocked by me
			map<Location, vector<MoveStep> >::iterator it;
			if ( (it = pending_moves.find(best_ant_curr_loc)) != pending_moves.end() ) {
				MoveStep& best_step_to_best_ant = GetBestBlockedMove(it->second);
				if (best_step_to_best_ant.task.ant_id == blocking_ant.GetAntId()) {
					SendMoveOrder(best_ant, best_step.move_dir);
					SendMoveOrder(blocking_ant, best_step_to_best_ant.move_dir);
					resolved_blocking_loc_set.insert(best_ant_curr_loc);
					return;
				}
			}

			//// the ant is blocked by someone else, not by me
			//Location new_blocking_loc = blocking_ant.GetNextLocation();
			//if (new_blocking_loc != blocking_ant.GetCurrentLocation() && 
			//	resolved_blocking_loc_set.find(new_blocking_loc) != resolved_blocking_loc_set.end()) {
			//	ResolvePendingMove(new_blocking_loc);
			//	if (!IsBlocked(blocking_loc)) {
			//		SendMoveOrder(ant_info_map[best_step.task.ant_id], best_step.move_dir);
			//		return;
			//	}
			//}
		}
	}

	// Check again for enemy hill razors
	if (best_step.task.task_type == Task::TASK_RAZE_HILL && 
		state.detected_enemy_hills.find(best_step.task.dest_loc) != state.detected_enemy_hills.end()) {
			AntInfo& my_ant = ant_info_map[best_step.task.ant_id];
			my_ant.FinishTask();
			my_ant.SetHillTarget(best_step.task.dest_loc);
			my_ant.SetHillAttacker(true);
			MoveToEnemyHill(my_ant, best_step.task);
	}

	resolved_blocking_loc_set.insert(blocking_loc);
}

//
MoveStep& Bot::GetBestBlockedMove(vector<MoveStep>& steps) {
	int best_score = -9999;
	int best_task_index = 0;
	// A very subtle bug here if simply use "steps.size()" instead of "num_moves" below
	// The move steps list would keep growing if the second try in finding a path also fails
	int num_moves = steps.size();
	for (int i = 0; i < num_moves; i++) {
		state.bug << "Ant #" << steps[i].task.ant_id << " at" << ant_info_map[steps[i].task.ant_id].GetCurrentLocation() << "     ";;
		if (steps[i].task.score > best_score) {
			best_score = steps[i].task.score;
			best_task_index = i;
		}
	}
	state.bug << endl;

	return steps[best_task_index];
}

///
string Bot::GetTaskString(int task_type) {
	switch (task_type) {
	case Task::TASK_GET_FOOD:
		return "Hunt for Food";
	case Task::TASK_RAZE_HILL:
		return "Raze Enemy Hill";
	case Task::TASK_ATTACK_ENEMY:
		return "Intercept Enemy";
	case Task::TASK_EXPLORE_MAP:
		return "Explore Map";
	case Task::TASK_WANDERING:
		return "Wandering";
	default:
		return "Other";
	}
}

//
static const int MAX_TASK_DIS = 40;
static const int MAX_FOOD_HUNT_DIS = 40;
static const int FOOD_HUNT_SCORE_UNIT = 10000;
TaskInfo Bot::EvaluateHuntFoodTask(AntInfo& my_ant, const Location& food_loc) {
	TaskInfo task;
	task.ant_id = my_ant.GetAntId();
	task.dest_loc = food_loc;
	task.task_type = Task::TASK_GET_FOOD;
	task.score = 0;

	// don't get food if none of my hills is left
	if (state.my_remained_hills.size() == 0) {
		task.score = -99999;
		return task;
	}

	if (my_ant.GetFoodTarget() == food_loc) {
		task.score = (MAX_FOOD_HUNT_DIS - my_ant.GetDistanceToDestination() + 2) * FOOD_HUNT_SCORE_UNIT 
						 + state.tile_bonus_values[food_loc.row][food_loc.col] / 100; 
		return task;
	}
	else {
		Location my_loc = my_ant.GetCurrentLocation();
		task.path = FindShortestPath(my_loc, food_loc, my_ant.GetWanderDirection() >> 1, MAX_FOOD_HUNT_DIS);
		task.score = (MAX_FOOD_HUNT_DIS - task.path.size()) * FOOD_HUNT_SCORE_UNIT + 
						+ state.tile_bonus_values[food_loc.row][food_loc.col] / 100; ;
		return task;
	}
}


// Evaluate the score for intercepting an enemy ant
static const int MAX_INTERCEPT_DIS = 25;
static const int INTERCEPT_ENEMY_SCORE_UNIT = 1000;
static const int DEFEND_MY_HILL_SCORE_UNIT = 10000;
TaskInfo Bot::EvaluateInterceptEnemyTask(AntInfo& my_ant, const Location& enemy_loc) {
	TaskInfo task;
	task.ant_id = my_ant.GetAntId();
	task.task_type = Task::TASK_ATTACK_ENEMY;
	task.score = 0;

	AntInfo& enemy_ant = enemy_ant_info[enemy_loc];
	task.dest_loc = enemy_loc;  //enemy_ant.GetWeakestDefendLocation(); //
	/*if (enemy_ant.IsHillAttacker())
		task.dest_loc = enemy_loc;
	else 
		task.dest_loc = enemy_ant.GetWeakestDefendLocation();*/
	
	// Check if the enemy ant is worth intercepting
	Location my_loc = my_ant.GetCurrentLocation();
	vector<Location> my_fighters;
	my_fighters.insert(my_fighters.end(), my_defend_area_map[enemy_loc].begin(), my_defend_area_map[enemy_loc].end());
	my_fighters.insert(my_fighters.end(), my_backup_area_map[enemy_loc].begin(), my_backup_area_map[enemy_loc].end());
	
	int num_my_army = my_fighters.size();
	if (state.distance2(my_loc, enemy_loc) > MAX_INTERCEPT_DISTANCE2 
				|| (num_my_army == 0 && !enemy_ant.IsHillAttacker()) 
				|| (num_my_army >= GetMaxNumFireCover(enemy_ant.IsHillAttacker()))
		) {
		task.score = -99999;
		return task;
	}

	// Check if my ant was already tracking enemy in the previous turn
	if (my_ant.GetTaskType() == Task::TASK_ATTACK_ENEMY && !enemy_ant.IsHillAttacker()) {
		Location target_loc = my_ant.GetTaskDstLocation();
		if (!state.IsEnemyAnt(target_loc)) {
			int direction = 0;
			for (int d = 0; d < TDIRECTIONS; d++) {
				Location loc = state.getLocation(target_loc, d);
				if (state.IsEnemyAnt(loc)) {
					target_loc = loc;
					direction = d;
					break;
				}
			}

			vector<int> new_path = my_ant.GetRemainedPath();
			new_path.push_back(direction);
			my_ant.SetNewTask(Task::TASK_ATTACK_ENEMY, target_loc, new_path);
		}

		task.score = (MAX_TASK_DIS) * INTERCEPT_ENEMY_SCORE_UNIT;
		return task;
	}

	
	vector<int> path = FindShortestPath(my_loc, enemy_loc, my_ant.GetWanderDirection() >> 1, MAX_INTERCEPT_DIS);
	
	if (path.size() >= MAX_INTERCEPT_DIS) {
		task.score = -99999;
		return task;
	}
	else {
		AntInfo& enemy_ant = enemy_ant_info[enemy_loc];
		if (enemy_ant.IsHillAttacker()) {
			Location hill = enemy_ant.GetHillTarget();
			int enemy_dis_to_hill = state.xyDistance(enemy_loc, hill);
			int dis_to_enemy = state.xyDistance(my_ant.GetCurrentLocation(), enemy_loc);

			num_my_army = 0;
			for (int i = 0; i < my_fighters.size(); i++) {
				int dis_to_hill = state.xyDistance(my_fighters[i], hill);
				if (dis_to_hill < enemy_dis_to_hill)
					num_my_army++;
			}

			task.path = path;
			task.score = (MAX_TASK_DIS - dis_to_enemy - num_my_army * 5) * DEFEND_MY_HILL_SCORE_UNIT + 
				(MAX_TASK_DIS -enemy_dis_to_hill - num_my_army * 5) * INTERCEPT_ENEMY_SCORE_UNIT;
		}
		else {
			task.path = path;
			task.score = (MAX_TASK_DIS - path.size() - my_view_area_map[enemy_loc].size() - num_my_army * 5) * INTERCEPT_ENEMY_SCORE_UNIT;
		}
		return task;
	}
}



//
static const int RAZE_HILL_SCORE_UNIT = 50000;
static const int RAZE_HILL_TOUCHDOWN_SCORE = 50000; //100000;
TaskInfo Bot::EvaluateRazeHillTask(AntInfo& my_ant, const Location& hill_loc) {
	TaskInfo task;
	task.ant_id = my_ant.GetAntId();
	task.dest_loc = hill_loc;
	task.task_type = Task::TASK_RAZE_HILL;
	task.score = 0;

	Location my_loc = my_ant.GetCurrentLocation();
	double divider = state.xyDistance(my_loc, hill_loc) * 1.0 / (num_rows + num_cols) + 0.5;
	if (my_ant.GetHillTarget() == hill_loc) {
		if (divider < 0.6)
			task.score = RAZE_HILL_TOUCHDOWN_SCORE * 2;
		else
			task.score = RAZE_HILL_SCORE_UNIT * 2;
		return task;

		//task.score = //MAX_TASK_DIS * INTERCEPT_ENEMY_SCORE_UNIT;
		return task;
	}
	else {
		if (divider < 0.6)
			task.score = RAZE_HILL_TOUCHDOWN_SCORE / divider;
		else
			task.score = RAZE_HILL_SCORE_UNIT / divider;
		return task;
	}
}


// Evaluate a map exploration task
static const int MAP_EXPLORATION_SCORE_UNIT = 500;
static const int MIN_VISIBILITY_THRESH = 0;
static const int MAP_EXPLORER_THRESH = 0; //12;
TaskInfo Bot::EvaluateExploreMapTask(AntInfo& my_ant) {
	TaskInfo task;
	task.ant_id = my_ant.GetAntId();
	task.dest_loc = NULL_LOCATION;
	task.task_type = Task::TASK_EXPLORE_MAP;
	task.score = 0;

	Location my_loc = my_ant.GetCurrentLocation();
	int max_visibility_points = 0;
	int direction = 0;
	for (int d = 0; d < TDIRECTIONS; d++) {
		Location loc = state.getLocation(my_loc, d);
		if (!IsBlocked(loc)) {
			int points = GetIncreasedVisibility(my_ant, d);
			if (points > max_visibility_points) {
				max_visibility_points = points;
				direction = d;
			}
		}
	}

	if (max_visibility_points > MIN_VISIBILITY_THRESH) {
		task.dest_loc = state.getLocation(my_loc, direction);
		task.score = max_visibility_points * MAP_EXPLORATION_SCORE_UNIT;
	}

	return task;
}


// Evaluate a wandering move task
static const int WANDER_SCORE_UNIT = 100;
TaskInfo Bot::EvaluateWanderingTask(AntInfo& my_ant) {
	TaskInfo task;
	task.ant_id = my_ant.GetAntId();
	task.dest_loc = NULL_LOCATION;
	task.task_type = Task::TASK_WANDERING;
	task.score = WANDER_SCORE_UNIT;

	// Determine the wandering destination for my ant
	Location my_loc = my_ant.GetCurrentLocation();
	if (my_ant.IsWandering() && my_ant.GetNextLocation() != my_loc) {
		task.dest_loc = my_ant.GetTaskDstLocation();
	}
	else {
		task.dest_loc = GetNextWanderingLocation(my_ant);
	}

	return task;
}


// Get the next wandering destination for my ant
Location Bot::GetNextWanderingLocation(AntInfo& my_ant) {
	int wander_dir = my_ant.GetWanderDirection();
	int row_offset = WANDER_DIRECTIONS[wander_dir][0];
	int col_offset = WANDER_DIRECTIONS[wander_dir][1];
	Location target = my_ant.GetCurrentLocation();
	target = state.getLocation(target, row_offset * state.viewradius, col_offset * state.viewradius);
	while (IsBlocked(target)) {
		target = state.getLocation(target, row_offset, col_offset);
	}

	return target;
}


//
bool Bot::IsTaskValid(TaskInfo& task) {
	switch (task.task_type) {
	case Task::TASK_GET_FOOD:
		if (food_hunters[task.dest_loc].size() == 0)
			return true;
		else
			return false;
		/*if (food_hunters[task.dest_loc].size() > 0 && food_hunters[task.dest_loc][0] == task.ant_id)
			return true;
		else
			return false;*/
	case Task::TASK_ATTACK_ENEMY:
		if (num_interceptors < max_num_interceptors && num_remained_idle_ants > 3 /*(num_interceptors + num_razers) < (num_idle_ants * 3 / 4)*/  
													/*&& (num_interceptors + num_razers) > 8*/ )
			return true;
		else
			return false;

	case Task::TASK_RAZE_HILL:
		if ( (num_razers < max_num_razers || 
				state.distance2(task.dest_loc, ant_info_map[task.ant_id].GetCurrentLocation()) <= MAX_PATH_SEARCH_DISTANCE2) 
								&& num_remained_idle_ants > 3 )
			return true;
		else
			return false;

	default:
		return true;
	}
}



// Execute a task by making the move to the next location
void Bot::ExecuteTask(TaskInfo& task) {
	switch (task.task_type) {
	case Task::TASK_GET_FOOD:
		ExecuteHuntFoodTask(task);
		break;
	case Task::TASK_ATTACK_ENEMY:
		ExecuteInterceptEnemyTask(task);
		break;
	case Task::TASK_RAZE_HILL:
		ExecuteRazeEnemyHillTask(task);
		break;
	default:
		break;
	}

	num_remained_idle_ants--;
}

//
void Bot::ExecuteHuntFoodTask(TaskInfo& task) {
	AntInfo& my_ant = ant_info_map[task.ant_id];
	//if (my_ant.HasMoved())
	//	return;

	Location my_loc = my_ant.GetCurrentLocation();

	food_hunters[task.dest_loc].push_back(my_ant.GetAntId());
	num_food_hunters++;

	// If it's the same task as the one the ant currently has, then just do the next move
	if (my_ant.GetTaskDstLocation() == task.dest_loc && my_ant.GetTaskType() == Task::TASK_GET_FOOD) {
		MoveToTarget(my_ant, task);
	}
	else {	// else assign a new task
		my_ant.FinishTask();
		my_ant.SetFoodTarget(task.dest_loc);
		my_ant.SetNewTask(Task::TASK_GET_FOOD, task.dest_loc, task.path);
		MoveToNextLocation(my_ant, my_ant.GetNextMove(), task);
		//MoveToTarget(my_ant, task);
	}

}

//
void Bot::ExecuteInterceptEnemyTask(TaskInfo& task) {
	AntInfo& my_ant = ant_info_map[task.ant_id];
	Location my_loc = my_ant.GetCurrentLocation();

	enemy_intercepters[task.dest_loc].push_back(my_ant.GetAntId());
	num_interceptors++;

	// If it's the same task as the one the ant currently has, then just do the next move
	if (my_ant.GetTaskDstLocation() == task.dest_loc && my_ant.GetTaskType() == Task::TASK_ATTACK_ENEMY) {
		MoveToTarget(my_ant, task);
	}
	else {	// else assign a new task
		my_ant.FinishTask();

		my_ant.SetNewTask(Task::TASK_GET_FOOD, task.dest_loc, task.path);
		MoveToTarget(my_ant, task);
	}
}


//
void Bot::ExecuteRazeEnemyHillTask(TaskInfo& task) {
	AntInfo& my_ant = ant_info_map[task.ant_id];
	Location my_loc = my_ant.GetCurrentLocation();

	hill_razers[task.dest_loc].push_back(my_ant.GetAntId());
	num_razers++;

	if (my_ant.GetHillTarget() == task.dest_loc && !state.IsWater(my_ant.GetTaskDstLocation()) ) {
		MoveToEnemyHill(my_ant, task);	
	}
	else {
		my_ant.FinishTask();
		my_ant.SetHillTarget(task.dest_loc);
		my_ant.SetHillAttacker(true);
		MoveToEnemyHill(my_ant, task);
	}
}


// Move to an enemy hill
void Bot::MoveToEnemyHill(AntInfo& my_ant, TaskInfo& task) {
	Location target_hill = my_ant.GetHillTarget();
	TaskInfo raze_hill_task = task;

	Location my_loc = my_ant.GetCurrentLocation();
	if (my_loc != my_ant.GetNextLocation()) {
		raze_hill_task.dest_loc = my_ant.GetTaskDstLocation();
		MoveToTarget(my_ant, raze_hill_task);
		if (my_ant.IsSneaker())
			sneakers[target_hill].push_back(my_ant.GetAntId());

		return;
	}
	else {
		vector<int> new_path;
		new_path = FindShortestPath(my_ant.GetCurrentLocation(), target_hill, my_ant.GetWanderDirection() >> 1, MAX_PATH_STEPS, true);
		if (new_path.size() == 0) {
			new_path = FindShortestPath(my_ant.GetCurrentLocation(), target_hill, my_ant.GetWanderDirection() >> 1);
		}
		
		my_ant.SetNewTask(Task::TASK_RAZE_HILL, target_hill, new_path);
		MoveToNextLocation(my_ant, my_ant.GetNextMove(), task);
		//SetTargetAndMove(my_ant, target_hill);
		return;
	}
}

//
void Bot::DoMapExplorationOrWandering(AntInfo& my_ant) {
	state.bug << "***** Do move for Ant #" << my_ant.GetAntId() << " at " << my_ant.GetCurrentLocation() << " *****" << endl; 
	
	if (my_ant.GetTaskType() != Task::TASK_WANDERING && my_ant.GetTaskType() != Task::TASK_EXPLORE_MAP)
		my_ant.FinishTask();

	if (my_ant.GetNextLocation() != my_ant.GetCurrentLocation() && !state.IsWater(my_ant.GetTaskDstLocation()) ) {
		TaskInfo task;
		task.ant_id = my_ant.GetAntId();
		task.dest_loc = my_ant.GetTaskDstLocation();
		task.score = WANDER_SCORE_UNIT;
		task.task_type = my_ant.GetTaskType();

		MoveToTarget(my_ant, task);

		if (my_ant.GetTaskType() == Task::TASK_WANDERING)
			num_wanderers++;
		else
			num_map_explorers++;

		return;
	}
	else {
		my_ant.FinishTask();
		DoWandering(my_ant);
	}
}




// Explore the map
bool Bot::DoMapExploration(AntInfo& my_ant) {
	Location my_loc = my_ant.GetCurrentLocation();

	int max_visibility_points = 0;
	int direction = 0;
	int wander_dir = my_ant.GetWanderDirection();
	for (int d = 0; d < TDIRECTIONS; d++) {
		int dir = (d + wander_dir) & 0x03;
		Location loc = state.getLocation(my_loc, dir);
		if (!IsBlocked(loc) && !IsMapExplorationBlocked(my_loc, dir)) {
			int points = GetIncreasedVisibility(my_ant, dir);
			if (points > max_visibility_points) {
				max_visibility_points = points;
				direction = dir;
			}
		}
	}

	if (max_visibility_points > MIN_VISIBILITY_THRESH) { // false) { //
		state.bug << "I will do wandering at direction " << CDIRECTIONS[direction] << endl;
		my_ant.SetTaskType(Task::TASK_EXPLORE_MAP);
		//if (max_visibility_points > MAP_EXPLORER_THRESH)
		my_ant.SetMapExplorer(true);
		SendMoveOrder(my_ant, direction);
		num_map_explorers++;

		// Update visibility map
		vector<LocationOffset>& new_offsets = new_visibility_offsets[direction];
		for (int i = 0; i < new_offsets.size(); i++) {
			Location loc = state.getLocation(my_loc, new_offsets[i].row_offset, 
				new_offsets[i].col_offset);
			my_visibility_map[loc.row][loc.col]++;
		}

		vector<LocationOffset>& erased_offsets = new_visibility_offsets[(direction + 2) % TDIRECTIONS];
		for (int i = 0; i <erased_offsets.size(); i++) {
			Location loc = state.getLocation(my_loc, erased_offsets[i].row_offset, 
				erased_offsets[i].col_offset);
			my_visibility_map[loc.row][loc.col]--;
		}

		return true;
	}
	else
		return false;
}


// Just wander
bool Bot::DoWandering(AntInfo& my_ant) {
	Location my_loc = my_ant.GetCurrentLocation();
	Location grid_loc = state.GetClosestUnvisitedGridLocation(my_loc, my_ant.GetWanderDirection());
	if (DoMapExploration(my_ant)) {
		return true;
	}
	else {
		my_ant.FinishTask();

		//Location grid_loc = state.GetClosestUnvisitedGridLocation(my_loc, my_ant.GetWanderDirection());
		Location grid_loc = state.GetClosestInvisibleGridLocation(my_loc, my_ant.GetWanderDirection());
		if (!state.IsVisible(grid_loc)) {
			TaskInfo task;
			task.ant_id = my_ant.GetAntId();
			task.dest_loc = grid_loc;
			task.score = WANDER_SCORE_UNIT;
			task.task_type = Task::TASK_WANDERING;
			my_ant.SetWandering(true);

			MoveToTarget(my_ant, task);
			num_wanderers++;

			state.SetSentinel(grid_loc, my_ant.GetWanderDirection());
			return true;
		}
		else {
			int wander_dir = my_ant.GetWanderDirection();
			state.bug << "I will do wandering at direction " << wander_dir << endl;
			int row_offset = WANDER_DIRECTIONS[wander_dir][0];
			int col_offset = WANDER_DIRECTIONS[wander_dir][1];

			Location target = my_ant.GetCurrentLocation();
			target = state.getLocation(target, row_offset * 1, col_offset * 1);
			while (state.territory_map[target.row][target.col] == WATER) {
				target = state.getLocation(target, row_offset, col_offset);
			}

			TaskInfo task;
			task.ant_id = my_ant.GetAntId();
			task.dest_loc = target;
			task.score = WANDER_SCORE_UNIT;
			task.task_type = Task::TASK_WANDERING;
			my_ant.SetWandering(true);
			MoveToTarget(my_ant, task);
			num_wanderers++;

			return true;
		}
		
	}
}



// Get the total degrees of increased visibility for an ant with a proposed move
int	Bot::GetIncreasedVisibility(AntInfo& my_ant, int direction) {
	Location my_loc = my_ant.GetCurrentLocation();

	int unvisited_score = (state.unvisited_grid.size() > (state.total_num_grids / 2) ) ? 5 : 1;
	int visited_score = (state.unvisited_grid.size() > (state.total_num_grids / 2) ) ? 1 : 2;
	int new_visibility = 0;
	vector<LocationOffset>& new_offsets = new_visibility_offsets[direction];
	int size_offsets = new_offsets.size();
	for (int i = 0; i < size_offsets; i++) {
		Location loc = state.getLocation(my_loc, new_offsets[i].row_offset, 
			new_offsets[i].col_offset);
		if (my_visibility_map[loc.row][loc.col] == 0 && !state.IsWater(loc)) {
			if (state.unvisited_map.find(loc) != state.unvisited_map.end())
				new_visibility += unvisited_score;
			else
				new_visibility += visited_score;
		}
	}

	int erased_visibility = 0;
	vector<LocationOffset>& erased_offsets = new_visibility_offsets[(direction + 2) % TDIRECTIONS ];
	size_offsets = erased_offsets.size();
	for (int i = 0; i < size_offsets; i++) {
		Location loc = state.getLocation(my_loc, erased_offsets[i].row_offset, 
			erased_offsets[i].col_offset);
		if (my_visibility_map[loc.row][loc.col] == 1) 
			erased_visibility++;
	}

	return (new_visibility - erased_visibility);
}


//
static const int LOOK_AHEAD_STEPS = 8;
bool Bot::IsMapExplorationBlocked(Location& my_loc, int wander_dir) {
	Location look_ahead_loc = my_loc;
	int dir1 = (wander_dir + 1) & 0x03;
	int dir2 = (wander_dir + 3) & 0x03;

	for (int i = 0; i < LOOK_AHEAD_STEPS; i++) {
		look_ahead_loc = state.getLocation(look_ahead_loc, wander_dir);
		if (state.IsWater(look_ahead_loc) || state.IsMyAnt(look_ahead_loc))
			return true;
		else if (!IsWanderBlocked(look_ahead_loc, dir1) || !IsWanderBlocked(look_ahead_loc, dir2))
			return false;
	}

	return false;
}

//
bool Bot::IsWanderBlocked(Location& my_loc, int wander_dir) {
	Location look_ahead_loc = my_loc;
	for (int i = 0; i < LOOK_AHEAD_STEPS; i++) {
		look_ahead_loc = state.getLocation(look_ahead_loc, wander_dir);
		if (state.IsWater(look_ahead_loc) || state.IsMyAnt(look_ahead_loc))
			return true;
	}

	return false;
}



// Get a best-balanced wander direction for an ant
const static int MAX_WANDER_NUM = 3;
int	Bot::GetWanderDirection(Location ant_loc) {
	int min = 9999;
	int min_dir = 0;
	for (int i = 0; i < NUM_WANDER_DIRECTIONS; i++) {
		if (num_wandering_ants[i] < min) {
			min = num_wandering_ants[i];
			min_dir = i;
		}
	}
	return min_dir;
}

///
int Bot::GetMaxNumFireCover(bool is_hill_attacker) {
	if (is_hill_attacker)
		return 5;
	else if (state.invisible_grid.size() <= (state.total_num_grids / 2) )
		return 2;
	else
		return 4; // 2; //
}


///
int	Bot::GetMaxRazeGroupSize() {
	if (state.my_remained_hills.size() > 0)
		return state.myAnts.size() / 6 + 2;
	else
		return state.myAnts.size();
}


int Bot::GetTotalNumRazers() {
	int num_razers = 0; 
	set<Location>::iterator it;
	for (it = state.detected_enemy_hills.begin(); it != state.detected_enemy_hills.end(); it++) {
		num_razers += hill_razers[*it].size(); 
	}

	return num_razers;
}


int  Bot::GetMaxTotalRazerSize() {
	if (state.my_remained_hills.size() > 0) {
		return max( (num_idle_ants - state.invisible_grid.size()) 
					- state.enemyAnts.size() * 2, (num_idle_ants * 1 / 4) + 1);
	}
	else {
		return num_idle_ants;
	}
}


int	Bot::GetMaxIncerceptorSize() {
	//return max( (num_idle_ants - state.invisible_grid.size()) / 2, num_idle_ants * 1 / 4 + 1);
	return min(state.enemyAnts.size() * 2,  num_idle_ants * 1 / 4 + 1);
	//return (state.myAnts.size() >> 1) + 2;
}


int  Bot::GetMaxNumMapExplorer() {
	return num_idle_ants / 9;
}


int	 Bot::GetMaxNumWanderer() {

}

// Set a target for an ant, and make the move for this turn
void Bot::MoveToTarget(AntInfo& my_ant, TaskInfo& task) {
	state.bug << "Set target for ant " << my_ant.GetAntId() << 
		".    Current: " << my_ant.GetCurrentLocation() << "    Target: " << task.dest_loc << endl;

	int d;
	if (task.dest_loc == my_ant.GetTaskDstLocation() && my_ant.IsPathStillValid()) {	// I'm on the middle-way
		d = my_ant.GetNextMove();
	}
	else {	// This is a new target for me
		vector<int> path = FindShortestPath(my_ant.GetCurrentLocation(), 
										task.dest_loc, my_ant.GetWanderDirection() >> 1);
		my_ant.SetNewTask(task.task_type, task.dest_loc, path);

		d = my_ant.GetNextMove();
	}

	MoveToNextLocation(my_ant, d, task);
}


// Move to the next location (conditionally, need to avoid collision)
void Bot::MoveToNextLocation(AntInfo& my_ant, int d, TaskInfo& task) {
	if (! (d >= 0  && d < TDIRECTIONS))
		return;

	Location my_loc = my_ant.GetCurrentLocation();
	Location next_loc = state.getLocation(my_loc, d);
	map<Location, int>::iterator it;
	if (!IsBlocked(next_loc)) {
		SendMoveOrder(my_ant, d);
	}
	else {
		if (next_loc == my_ant.GetTaskDstLocation()) {
			state.bug << "I'm blocked by the guy at " << next_loc << "!" << endl;
			MoveStep step;
			step.move_dir = d;
			step.task = task;
			pending_moves[next_loc].push_back(step);
			return;
		}

		vector<int> path = FindShortestPath(my_loc, my_ant.GetTaskDstLocation(), my_ant.GetWanderDirection());
		if (path.size()> 0) {
			my_ant.SetNewTask(my_ant.GetTaskType(), my_ant.GetTaskDstLocation(), path);
			SendMoveOrder(my_ant, my_ant.GetNextMove());
		}
		else {
			state.bug << "I'm blocked by the guy at " << next_loc << "!" << endl;
			MoveStep step;
			step.move_dir = d;
			step.task = task;
			pending_moves[next_loc].push_back(step);
		}
	}
}

// Check whether a location is blocked by water, food, enemy ant, 
// my ant currently in the location, or my ant that has moved to 
// this location in this turn
bool Bot::IsBlocked(Location loc) {
	if (state.IsWater(loc) || state.IsFood(loc) || state.IsEnemyAnt(loc) ||
		(next_ant_id_map.find(loc) != next_ant_id_map.end()) ||
		curr_ant_id_map.find(loc) != curr_ant_id_map.end() )
		return true;
	else
		return false;
}



// Send orders to move my ants
void Bot::SendMoveOrders(set<Location>& my_ants, vector<int>& directions){
	if (my_ants.size() != directions.size()) 
		state.bug << "SendMoveOrders(): My ant size not equal to direction size!" << endl;

	set<Location>::iterator it;
	int i = 0;
	for (it = my_ants.begin(); it != my_ants.end(); it++) {
		SendMoveOrder(ant_info_map[start_ant_id_map[*it]], directions[i++]);
	}
}


// Send the actual order to move my ant
void Bot::SendMoveOrder(AntInfo& my_ant, int direction) {
	if (direction == MOVE_STAY) {
		my_ant.FinishTurnWithoutMove();
		next_ant_id_map[my_ant.GetCurrentLocation()] = my_ant.GetAntId();
	}
	else if (direction >= 0 && direction < TDIRECTIONS && !my_ant.HasMoved()) {
		Location my_loc = my_ant.GetCurrentLocation();
		Location new_loc = state.getLocation(my_loc, direction);
		
		my_ant.MakeMove(direction);
		next_ant_id_map[new_loc] = my_ant.GetAntId();
		curr_ant_id_map.erase(my_loc);
		curr_ant_id_map[new_loc] = my_ant.GetAntId();

		state.bug << "Move made for ant " << my_ant.GetAntId() 
			<< ".    Next Location: " << my_ant.GetCurrentLocation() << endl;

		if (state.IsWater(new_loc)) {
			state.bug << "I'm moving to a water location!" << endl;
		}
	}
}


// Update information for my ants
void Bot::UpdateMyAntsStatus() {
	const vector<Location>& my_ants = state.myAnts;
	int num_my_ants = my_ants.size();
	for (int i = 0; i < num_my_ants; i++) {
		Location my_ant_loc = my_ants[i];

		// If the ant has not been encountered before, then create a new ant identity
		map<Location, int>::iterator id_iter = curr_ant_id_map.find(my_ant_loc);
		if (id_iter == curr_ant_id_map.end()) {
			ant_info_map.insert(pair<int, AntInfo>(curr_ant_id, AntInfo(state, curr_ant_id)));
			ant_info_map[curr_ant_id].SetWanderDirection(GetWanderDirection(my_ant_loc));
			curr_ant_id_map.insert(pair<Location, int>(my_ant_loc, curr_ant_id));
			id_iter = curr_ant_id_map.find(my_ant_loc);
			curr_ant_id++;
		}

		// Find enemies within attack area and view area for my ant
		AntInfo& my_ant = ant_info_map[id_iter->second];
		my_ant.SetCurrentLocation(id_iter->first);
		UpdateMyAntInfo(my_ant);
	}

	start_ant_id_map = curr_ant_id_map;
	next_ant_id_map.clear();

	state.bug << "My Ant Analysis Time: " << state.timer.getTime() << "ms" << endl << endl;
}


// Update information for enemy ants
void Bot::UpdateEnemyAntsStatus() {
	enemy_ant_info.clear();

	const vector<Location>& enemy_ants = state.enemyAnts;
	int num_enemy_ants = enemy_ants.size();
	for (int e = 0; e < num_enemy_ants; e++) {
		Location enemy_loc = enemy_ants[e];
		enemy_ant_info.insert(pair<Location, AntInfo>(enemy_loc, 
			AntInfo(state, enemy_loc, state.grid[enemy_loc.row][enemy_loc.col].ant)));

		UpdateEnemyAntInfo(enemy_ant_info[enemy_loc]);
	}

	state.bug << "Enemy Ant Analysis Time: " << state.timer.getTime() << "ms" << endl << endl;
}


// Update information for my ant, e.g., enemies
void Bot::UpdateMyAntInfo(AntInfo& my_ant) {
	Location my_loc = my_ant.GetCurrentLocation();

	// Determine if I'm a hill defender 
	my_ant.SetHillDefender(false);
	vector<Location>& my_hills = state.myHills;
	int num_my_hills = my_hills.size();
	for (int i = 0; i < num_my_hills; i++) {
		int dis2 = state.distance2(my_loc, my_hills[i]);
		if ( dis2 <= DEFENSE_DISTANCE2 //INNER_DEFENSE_DISTANCE2 
			/*|| (dis2 > OUTER_DEFENSE_DISTANCE2_MIN && dis2 < OUTER_DEFENSE_DISTANCE2_MAX)*/) {
				my_ant.SetHillDefender(true);
				break;
		}
	}

	my_ant.SetMoved(false);
}


// Update information for an enemy ant
void Bot::UpdateEnemyAntInfo(AntInfo& enemy_ant) {
	int player_id = enemy_ant.GetPlayerId();
	Location ant_loc = enemy_ant.GetCurrentLocation();


	// Determine if the enemy is about to attack my hill
	enemy_ant.SetHillAttacker(false);
	vector<Location>& my_hills = state.myHills;
	int num_my_hills = my_hills.size();
	for (int i = 0; i < num_my_hills; i++) {
		int dis2_to_hill = state.distance2(ant_loc, my_hills[i]);
		if (dis2_to_hill <= DEFENSE_DISTANCE2) {
			enemy_ant.SetHillAttacker(true);
			if (state.distance2(ant_loc, my_hills[i]) < 
				state.distance2(ant_loc, enemy_ant.GetHillTarget()))
				enemy_ant.SetHillTarget(my_hills[i]);

			if (dis2_to_hill <= 4)
				enemy_ant.SetTouchDown(true);
		}
	}


	// Calculate the weakest defend location for the enemy ant
	set<LocationOffset>::iterator it;
	Location weakest_loc = NULL_LOCATION;
	int max_num_fighters = -9999;
	int weakest_dir = 0;
	Location loc;
	for (int d = 0; d < TDIRECTIONS; d++) {
		int num_fighters = 0;
		for (it = defend_offsets[d].begin(); it != defend_offsets[d].end(); it++) {
			loc = state.getLocation(ant_loc, it->row_offset, it->col_offset);
			if (state.IsMyAnt(loc))
				num_fighters++;
		}

		for (it = backup_offsets[d].begin(); it != backup_offsets[d].end(); it++) {
			loc = state.getLocation(ant_loc, it->row_offset, it->col_offset);
			if (state.IsMyAnt(loc))
				num_fighters++;
		}

		if (num_fighters > max_num_fighters) {
			weakest_dir = d;
			max_num_fighters = num_fighters;
		}
	}

	weakest_loc = state.getLocation(ant_loc, DIRECTIONS[weakest_dir][0] * 2, DIRECTIONS[weakest_dir][1] * 2);
	while (state.territory_map[weakest_loc.row][weakest_loc.col] == WATER) {
		weakest_loc = state.getLocation(weakest_loc, (weakest_dir + 2) % TDIRECTIONS);
	}

	enemy_ant.SetWeakestDefendLocation(weakest_loc);
}




//finishes the turn
void Bot::endTurn()
{
    if(state.turn > 0)
        state.reset();
    state.turn++;

    cout << "go" << endl;
};


// Setup information at the beginning of the game
void Bot::SetupGame() {
	curr_ant_id = 1;
	safe_turn_time_limit = state.turntime - state.turntime / 10;
	battle_time_limit = state.turntime - state.turntime / 30;

	num_rows = state.rows;
	num_cols = state.cols;
	for (int i = 0; i < num_rows; i++) {
		my_visibility_map.push_back(vector<int>(num_cols, 0));
	}

	for (int i = 0; i < NUM_WANDER_DIRECTIONS; i++) {
		num_wandering_ants.push_back(0);
	}

	// Generate the location offset maps within which an ant can attack
	map<LocationOffset, int> temp_map;
	for (int i = -state.attackradius; i <= state.attackradius; i++)
		for (int j = -state.attackradius; j <= state.attackradius; j++) {
			if ((i * i + j * j) <= state.attackradius2) {
				LocationOffset offset(i, j);
				attack_offsets.insert(offset);
				temp_map[offset] = 1;
			}
		}

    // Generate location offset for the defend area
	for (int d = 0; d < TDIRECTIONS; d++) {
		defend_offsets.push_back(set<LocationOffset>());
		backup_offsets.push_back(set<LocationOffset>());
	}

	set<LocationOffset>::iterator it;
	map<LocationOffset, int> tmp_defend_map;
	for (it = attack_offsets.begin(); it != attack_offsets.end(); it++) {
		for (int d = 0; d < TDIRECTIONS; d++) {
			LocationOffset offset(it->row_offset + DIRECTIONS[d][0], 
								  it->col_offset + DIRECTIONS[d][1]);

			if (temp_map[offset] != 1) {
				defend_offsets[d].insert(offset);
				tmp_defend_map[offset] = 1;
				//temp_map[offset] = 1;
			}
		}
	}
	temp_map.insert(tmp_defend_map.begin(), tmp_defend_map.end());

	// Generate location offset for the backup area
	set<LocationOffset>::iterator set_it;
	for (int d = 0; d < TDIRECTIONS; d++)
			for (int d2 = 0; d2 < TDIRECTIONS; d2++) {
				for (set_it = defend_offsets[d2].begin(); set_it != defend_offsets[d2].end(); set_it++) {
					LocationOffset offset = LocationOffset(set_it->row_offset + DIRECTIONS[d][0], 
														   set_it->col_offset + DIRECTIONS[d][1]);

					if (temp_map[offset] != 1 && tmp_defend_map[offset] != 1) {
						backup_offsets[d].insert(offset);
					}
		}
	}

	// Initialize defend and backup offsets in the BattleResolver class
	BattleResolver::attack_offsets.insert(attack_offsets.begin(), attack_offsets.end());
	for (int d = 0; d < TDIRECTIONS; d++) {
		BattleResolver::defend_offsets.insert(defend_offsets[d].begin(), defend_offsets[d].end());
		BattleResolver::backup_offsets.insert(backup_offsets[d].begin(), backup_offsets[d].end());
	}

	// Generate the location offset maps within which an ant can view
	map<LocationOffset, int> view_map;
	for (int i = -state.viewradius; i <= state.viewradius; i++)
		for (int j = -state.viewradius; j <= state.viewradius; j++) {
			if ((i * i + j * j) <= state.viewradius2) {
				LocationOffset offset = LocationOffset(i, j);
				view_offsets.insert(offset);
				view_map[offset] = 1;
			}
		}

	// Generate the location offsets for the visibility change mappings
	new_visibility_offsets = vector< vector<LocationOffset> >(TDIRECTIONS, vector<LocationOffset>());
	erased_visibility_offsets = vector< vector<LocationOffset> >(TDIRECTIONS, vector<LocationOffset>());
	for (int d = 0; d < TDIRECTIONS; d++) {
		map<LocationOffset, int> tmp_view_map = view_map;
		for (it = view_offsets.begin(); it != view_offsets.end(); it++) {
			LocationOffset view_offset = *it;
			LocationOffset new_offset = LocationOffset(view_offset.row_offset + DIRECTIONS[d][0], 
													view_offset.col_offset + DIRECTIONS[d][1]);
			if (tmp_view_map[new_offset] == 0) {
				new_visibility_offsets[d].push_back(new_offset);
			}
			else {
				tmp_view_map.erase(new_offset);
			}
		}

		for (it = view_offsets.begin(); it != view_offsets.end(); it++) {
			if (tmp_view_map[*it] == 1) {
				erased_visibility_offsets[d].push_back(*it);
			}
		}
	}

}

// Update map information at the beginning of each turn
void Bot::UpdateMapInfo() {
	// clear the visibility map
	for (int  i = 0; i < num_rows; i++) 
		for (int j = 0; j < num_cols; j++) {
			my_visibility_map[i][j] = 0;
		}

	// Update the number of wandering ants in each direction
	for (int i = 0; i < NUM_WANDER_DIRECTIONS; i++) {
		num_wandering_ants[i] = 0;
	}
	const vector<Location>& my_ants = state.myAnts;
	for (int i = 0; i < my_ants.size(); i++) {
		if (curr_ant_id_map.find(my_ants[i]) != curr_ant_id_map.end() ) {
			int ant_id = curr_ant_id_map[my_ants[i]];
			num_wandering_ants[ant_info_map[ant_id].GetWanderDirection()]++;
		}
	}

	UpdateMyBattleFieldInfo();
	UpdateEnemyBattleFieldInfo();

	BattleResolver::my_attack_area_map = my_attack_area_map;
	BattleResolver::my_defend_area_map = my_defend_area_map;
	BattleResolver::my_backup_area_map = my_backup_area_map;
	BattleResolver::enemy_attack_area_map = enemy_attack_area_map;
	BattleResolver::enemy_defend_area_map = enemy_defend_area_map;
	BattleResolver::enemy_backup_area_map = enemy_backup_area_map;
	
	//////////////////////////////////////////////////////////////////////////
	sneakers.clear();
	food_hunters.clear();
	hill_razers.clear();
	enemy_intercepters.clear();
	ant_tasks.clear();
	ant_task_vectors.clear();
	pending_moves.clear();

	num_interceptors = 0;
	num_food_hunters = 0;
	num_razers = 0;
	num_map_explorers = 0;
	num_wanderers = 0;
}


//
void Bot::UpdateMyBattleFieldInfo() {
	// Update the attack and alarm area maps for my ants and enemy ants
	my_attack_area_map.clear();
	my_defend_area_map.clear();
	my_backup_area_map.clear();
	my_view_area_map.clear();

	const vector<Location>& my_ants = state.myAnts;
	int size = my_ants.size();
	for (int i = 0; i < size; i++) {
		Location my_loc = my_ants[i];

		// Update visibility map
		set<LocationOffset>::iterator it;
		for (it = view_offsets.begin(); it != view_offsets.end(); it++) {
			Location loc = state.getLocation(my_loc, it->row_offset, it->col_offset);
			my_visibility_map[loc.row][loc.col]++;
		}

		// Update my attack area
		for (it = attack_offsets.begin(); it != attack_offsets.end(); it++) {
			Location loc = state.getLocation(my_loc, it->row_offset, it->col_offset);
			my_attack_area_map[loc].insert(my_loc);
		}


		// Update my defend and backup area
		for (int d = 0; d < TDIRECTIONS; d++) {
			Location loc = state.getLocation(my_loc, d);
			if (state.IsWater(loc)/*state.IsBlocked(loc)*/)
				continue;

			for (it = defend_offsets[d].begin(); it != defend_offsets[d].end(); it++) {
				Location new_loc = state.getLocation(my_loc, it->row_offset, it->col_offset);
				my_defend_area_map[new_loc].insert(my_loc);
			}

			for (it = backup_offsets[d].begin(); it != backup_offsets[d].end(); it++) {
				Location new_loc = state.getLocation(my_loc, it->row_offset, it->col_offset);
				my_backup_area_map[new_loc].insert(my_loc);
			}
		}


		for (it = view_offsets.begin(); it != view_offsets.end(); it++) {
			Location loc = state.getLocation(my_loc, it->row_offset, it->col_offset);
			my_view_area_map[loc].insert(my_loc);
		}
	}
}


//
void Bot::UpdateEnemyBattleFieldInfo() {
	enemy_attack_area_map.clear();
	enemy_defend_area_map.clear();
	enemy_backup_area_map.clear();
	enemy_view_area_map.clear();

	const vector<Location>& enemy_ants = state.enemyAnts;
	for (int i = 0; i < enemy_ants.size(); i++) {
		Location ant_loc = enemy_ants[i];
		// Update the enemy's attack area map
		set<LocationOffset>::iterator it;
		for (it = attack_offsets.begin(); it != attack_offsets.end(); it++) {
			Location loc = state.getLocation(ant_loc, it->row_offset, it->col_offset);
			enemy_attack_area_map[loc].insert(ant_loc);
		}

		// Update the enemy's defend line map
		for (int d = 0; d < TDIRECTIONS; d++) {
			Location loc = state.getLocation(ant_loc, d);
			if (state.IsWater(loc) /*state.IsBlocked(loc)*/)
				continue;

			for (it = defend_offsets[d].begin(); it != defend_offsets[d].end(); it++) {
				Location new_loc = state.getLocation(ant_loc, it->row_offset, it->col_offset);
				enemy_defend_area_map[new_loc].insert(ant_loc);
			}

			for (it = backup_offsets[d].begin(); it != backup_offsets[d].end(); it++) {
				Location new_loc = state.getLocation(ant_loc, it->row_offset, it->col_offset);
				enemy_backup_area_map[new_loc].insert(ant_loc);
			}
		}

		// Update enemy's view map 
		for (it = view_offsets.begin(); it != view_offsets.end(); it++) {
			Location new_loc = state.getLocation(ant_loc, it->row_offset, it->col_offset);
			my_view_area_map[new_loc].insert(ant_loc);
		}
	}
}



// Find the shortest path from a given source to a destination location
vector<int> Bot::FindShortestPath(const Location& src, const Location& dst, int start_direction, const int step_limit, const bool is_safe) {
	state.bug << "Start finding shortest path from " << src << " to " << dst << endl;
	if (src == dst)
		return vector<int>();

	vector<int> path = FindPathAStar(src, dst, start_direction, step_limit, is_safe);
	state.bug << "Shortest path found. Path length: " << path.size() << endl;
	//PrintPath(src, path);
	return path;
}


// Find the path from the source to a destination location using the A* algorithm
vector<int> Bot::FindPathAStar(const Location& src, const Location& dst, int start_direction, const int step_limit, const bool is_safe) {
	PathInfo init_path_info;
	init_path_info.curr_location = src;
	init_path_info.dis_to_target = state.xyDistance(src, dst);
	
	list<PathInfo> paths;
	paths.push_back(init_path_info);

	vector< vector<int> > visited_map(num_rows, vector<int>(num_cols, 0));
	list<PathInfo>::iterator it;
	list<PathInfo>::iterator pos_it;

	// Tested code
	while (paths.size() != 0) {
		PathInfo& path_info = *paths.begin();
		if (path_info.directions.size() > step_limit) {
			if (is_safe)
				return vector<int>();
			else
				return path_info.directions;
		}

		Location loc = path_info.curr_location;
		vector<PathInfo> new_path_nodes;
		for (int d = 0; d < TDIRECTIONS; d++) {
			int dir = (d + start_direction) & 0x03;
			Location next_loc = state.getLocation(loc, dir);
			if (next_loc == dst) {
				path_info.directions.push_back(dir);
				return path_info.directions;
			}

			if (state.territory_map[next_loc.row][next_loc.col] == WATER
				|| (state.grid[next_loc.row][next_loc.col].ant == 0) 
				|| (next_ant_id_map.find(next_loc) != next_ant_id_map.end())
				|| (is_safe && GetNumEnemiesInBattle(next_loc) > 0)
			) {
					continue;
			}	
			else if (visited_map[next_loc.row][next_loc.col] == 0) {
				visited_map[next_loc.row][next_loc.col] = 1;
				PathInfo new_path_info = path_info;
				new_path_info.curr_location = next_loc;
				new_path_info.directions.push_back(dir);
				new_path_info.dis_to_target = state.xyDistance(next_loc, dst);

				new_path_nodes.push_back(new_path_info);
			}
		}


		paths.pop_front();
		for (int i = 0; i < new_path_nodes.size(); i++) {
			pos_it = paths.end();
			for (it = paths.begin(); it != paths.end(); it++) {
				if (new_path_nodes[i].dis_to_target < it->dis_to_target) {
					pos_it = it;
					break;
				}
			}
			paths.insert(pos_it, new_path_nodes[i]);
		}
	}

	return vector<int>();
}


// DEBUG only: print out the path from source to destination 
void Bot::PrintPath(const Location& src, const vector<int>& path) {
	state.bug << src;
	Location cur_loc = src;
	for (int i = 0; i < path.size(); i++) {
		cur_loc = state.getLocation(cur_loc, path[i]); 
		state.bug << "->" << cur_loc;
	}
	state.bug << endl;
}


// Used to get the location that has the minimum distance to a specific location
template <class ForwardIterator>
ForwardIterator Bot::GetClosestLocation(Location loc, ForwardIterator first, ForwardIterator last ) {
	int min_dis = 99999999;
	ForwardIterator res = first;
	ForwardIterator it = first;
	while (it != last) {
		Location tmp_loc = static_cast<Location>(*it);
		int dis2 = state.distance2(loc, tmp_loc);
		if (dis2 < min_dis) {
			min_dis = dis2;
			res = it;
		}
		it++;
	}

	return res;
}
