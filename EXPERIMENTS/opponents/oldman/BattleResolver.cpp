//------------------------ BattleResolver.cpp -----------------------------------

#include "BattleResolver.h"

set<LocationOffset> BattleResolver::attack_offsets;
set<LocationOffset> BattleResolver::defend_offsets;
set<LocationOffset> BattleResolver::backup_offsets;
map<Location, set<Location> > BattleResolver::my_attack_area_map;
map<Location, set<Location> > BattleResolver::my_defend_area_map;
map<Location, set<Location> > BattleResolver::my_backup_area_map;
map<Location, set<Location> > BattleResolver::enemy_attack_area_map;
map<Location, set<Location> > BattleResolver::enemy_defend_area_map;
map<Location, set<Location> > BattleResolver::enemy_backup_area_map;

//
BattleResolver::BattleResolver(State& s, vector<AntInfo*>& my_ants, vector<AntInfo*>& enemy_ants, Location& enemy_loc):
				state(s), my_ants(my_ants), enemy_ants(enemy_ants), enemy_location(enemy_loc) {
	num_my_ants = my_ants.size();
	num_enemy_ants = enemy_ants.size();
	num_moved_ants = 0;

	for (int i = 0; i < num_my_ants; i++) {
		my_ant_locs.push_back(my_ants[i]->GetCurrentLocation());
		my_ant_loc_set.insert(my_ants[i]->GetCurrentLocation());
		if (my_ants[i]->HasMoved())
			num_moved_ants++;
	}

	for (int i = 0; i < num_enemy_ants; i++) {
		enemy_ant_locs.push_back(enemy_ants[i]->GetCurrentLocation());
		enemy_ant_loc_set.insert(enemy_ants[i]->GetCurrentLocation());
	}


	encourage_mutual_kill = false;
	/*if (num_my_ants > (num_enemy_ants + 1) )
		encourage_mutual_kill = true;*/

	encourage_mutual_kill_level = 0;
	if (state.myAnts.size() > 200) {
		encourage_mutual_kill = true;
		encourage_mutual_kill_level = 2;
	}

	for (int e = 0; e < num_enemy_ants; e++) {
		if (enemy_ants[e]->IsTouchDown() && num_my_ants >= num_enemy_ants) {
			encourage_mutual_kill = true;
			encourage_mutual_kill_level = 1;
			break;
		}
	}
	
	if (encourage_mutual_kill)
		state.bug << "Encourage mutual killing." << endl;
	else
		state.bug << "Not encourage mutual killing." << endl;


	has_defenders = false;
	for (int i = 0; i < num_my_ants; i++) {
		if (my_ants[i]->IsHillDefender()) {
			has_defenders = true;
			break;
		}
	}

	state.bug << "Number of my ants involved in the battle: " << num_my_ants << "    ";
	for (int i = 0; i < num_my_ants; i++) {
		state.bug << my_ant_locs[i] << ":" << (my_ants[i]->HasMoved() ? "Moved    " : "Not_Moved    ");
	}

	state.bug << endl << "Number of enemy ants involved in the battle: " << num_enemy_ants << "    ";
	for (int i = 0; i < num_enemy_ants; i++) {
		state.bug << enemy_ant_locs[i] << "  ";
	}
	state.bug << endl;
}


// Resolve a battle by choosing the "best" strategy
void BattleResolver::ResolveBattle() {
	if (num_my_ants > 8 || num_enemy_ants > 8) {
		state.bug << "Too many ants in battle. Using many-to-many battle strategy." << endl;
		ChooseManyToManyStrategy();
		return;
	}

	vector<MovePlan> my_move_plans;
	MovePlan my_init_plan;
	GenerateMovePlans(my_move_plans, my_init_plan, my_ant_locs, 0, true);
	state.bug << "Finish generating my move plans. Total number of plans: " << my_move_plans.size() << endl;

	//
	if (my_move_plans.size() > MAX_PLAN_LIMIT) {
		state.bug << "Too many move plans. Simple battle strategy will be used."<< endl;
		//ChooseManyToManyStrategy();
		ChooseSimpleBattleStrategy(my_move_plans);
	}
	else {
		vector<MovePlan> enemy_move_plans;
		MovePlan enemy_init_plan;
		GenerateMovePlans(enemy_move_plans, enemy_init_plan, enemy_ant_locs, 0, false);
		state.bug << "Finish generating enemy move plans. Total number of plans:" << enemy_move_plans.size() << endl;

		if (enemy_move_plans.size() > MAX_PLAN_LIMIT ||
			my_move_plans.size() * enemy_move_plans.size() >= MAX_MIN_MAX_SITUATIONS) {
			state.bug << "Too many total move situations. Simple battle strategy will be used."<< endl;
			ChooseSimpleBattleStrategy(my_move_plans);
		}
		else {
			ChooseMinMaxBattleStrategy(my_move_plans, enemy_move_plans);
		}
	}
}


// Use a simple battle strategy when there are too many ants involved
void BattleResolver::ChooseSimpleBattleStrategy(vector<MovePlan>& my_move_plans) {
	MovePlan enemy_plan;
	for (int e = 0; e < num_enemy_ants; e++) {
		enemy_plan.directions.push_back(MOVE_STAY);
		enemy_plan.new_locations.push_back(enemy_ants[e]->GetCurrentLocation());
	}

	// Evaluate my move plans against enemy's staying plan
	vector<MovePlan>::iterator my_it;
	int my_max_score = -9999999;
	vector<MovePlan>::iterator max_score_it = my_move_plans.begin();
	for (my_it = my_move_plans.begin(); my_it != my_move_plans.end(); my_it++) {
		int score = GetPlanScore(*my_it, enemy_plan);
		if (score > my_max_score) {
			my_max_score = score;
			max_score_it = my_it;
		}
	}
	
	if (max_score_it != my_move_plans.end())
		best_move_plan = *max_score_it;
}


//
void BattleResolver::ChooseManyToManyStrategy() {
	MovePlan my_defend_plan;
	set<Location> occupied_locations;
	set<Location> available_locations;

	for (int i =0; i < num_my_ants; i++) {
		Location my_loc = my_ants[i]->GetCurrentLocation();
		int dir = MOVE_STAY;
		Location move_to_loc = my_loc;
		int max_num_defenders = enemy_defend_area_map[my_loc].size();
		for (int d = 0; d < TDIRECTIONS; d++) {
			Location loc = state.getLocation(my_loc, d);
			if ( (state.IsBlocked(loc) && available_locations.find(loc) == available_locations.end()) 
					|| occupied_locations.find(loc) != occupied_locations.end())
				continue;

			if (enemy_defend_area_map[loc].size() > max_num_defenders && enemy_attack_area_map[loc].size() == 0) {
				max_num_defenders = enemy_defend_area_map[loc].size();
				dir = d;
				move_to_loc = loc;
			}
		}

		my_defend_plan.directions.push_back(dir);
		my_defend_plan.new_locations.push_back(move_to_loc);
		occupied_locations.insert(move_to_loc);
		if (dir != MOVE_STAY)
			available_locations.insert(my_loc);
	}


	// Attack plan
	MovePlan my_attack_plan;
	occupied_locations.clear();
	available_locations.clear();
	for (int i =0; i < num_my_ants; i++) {
		Location my_loc = my_ants[i]->GetCurrentLocation();
		int dir = MOVE_STAY;
		Location move_to_loc = my_loc;
		int max_num_attackers = enemy_attack_area_map[my_loc].size();
		for (int d = 0; d < TDIRECTIONS; d++) {
			Location loc = state.getLocation(my_loc, d);
			if ( (state.IsBlocked(loc) && available_locations.find(loc) == available_locations.end()) 
				|| occupied_locations.find(loc) != occupied_locations.end())
				continue;

			if (enemy_attack_area_map[loc].size() > max_num_attackers) {
				max_num_attackers = enemy_attack_area_map[loc].size();
				dir = d;
				move_to_loc = loc;
			}
		}

		my_attack_plan.directions.push_back(dir);
		my_attack_plan.new_locations.push_back(move_to_loc);
		occupied_locations.insert(move_to_loc);
		if (dir != MOVE_STAY)
			available_locations.insert(my_loc);
	}


	// Dodge plan
	MovePlan my_dodge_plan;
	occupied_locations.clear();
	available_locations.clear();
	for (int i =0; i < num_my_ants; i++) {
		Location my_loc = my_ants[i]->GetCurrentLocation();
		int dir = MOVE_STAY;
		Location move_to_loc = my_loc;
		for (int d = 0; d < TDIRECTIONS; d++) {
			Location loc = state.getLocation(my_loc, d);
			if ( (state.IsBlocked(loc) && available_locations.find(loc) == available_locations.end()) 
				|| occupied_locations.find(loc) != occupied_locations.end())
				continue;

			if (enemy_attack_area_map[loc].size() == 0 && enemy_defend_area_map[loc].size() == 0 
						&& enemy_backup_area_map[loc].size() > 0) {
				dir = d;
				move_to_loc = loc;
				break;
			}
		}

		my_dodge_plan.directions.push_back(dir);
		my_dodge_plan.new_locations.push_back(move_to_loc);
		occupied_locations.insert(move_to_loc);
		if (dir != MOVE_STAY)
			available_locations.insert(my_loc);
	}


	MovePlan enemy_plan;
	for (int e = 0; e < num_enemy_ants; e++) {
		enemy_plan.directions.push_back(MOVE_STAY);
		enemy_plan.new_locations.push_back(enemy_ants[e]->GetCurrentLocation());
	}

	
	int attack_thresh = num_enemy_ants + 1;
	if (encourage_mutual_kill_level == 2)
		attack_thresh = num_enemy_ants;
	
	int attack_score = GetPlanScore(my_attack_plan, enemy_plan);
	int defend_score = GetPlanScore(my_defend_plan, enemy_plan);
	int dodge_score = GetPlanScore(my_dodge_plan, enemy_plan);

	if ( (defend_score >= attack_score && defend_score >= dodge_score)) {
		best_move_plan = my_defend_plan;
	}
	else if (attack_score > defend_score && attack_score >= dodge_score && num_my_ants >= attack_thresh) {
		best_move_plan = my_attack_plan;
	}
	else
		best_move_plan = my_dodge_plan;
}

//
void BattleResolver::ChooseMinMaxBattleStrategy(vector<MovePlan>& my_move_plans, vector<MovePlan>& enemy_move_plans) {
	vector<MovePlan>::iterator my_it;
	vector<MovePlan>::iterator enemy_it;
	vector<MovePlan>::iterator max_score_it = my_move_plans.begin();
	int my_max_score = -999999999;
	int max_average_scaled_score = -999999999;

	// Find the optimum plan using alpha-beta pruning
	for (my_it = my_move_plans.begin(); my_it != my_move_plans.end(); my_it++) {
		int enemy_min_score = 99999999;
		int total_scaled_score = 0;
		for (enemy_it = enemy_move_plans.begin(); enemy_it != enemy_move_plans.end(); enemy_it++) {
			int score = GetPlanScore(*my_it, *enemy_it);
			total_scaled_score += (score >> 7);
			if (score < my_max_score) {
				enemy_min_score = -99999999;
				break;
			}
			else if (score < enemy_min_score) {
				enemy_min_score = score;
			}
		}

		// Calculate the average score across all enemy plans
		int average_scaled_score = total_scaled_score / enemy_move_plans.size();

		if (enemy_min_score > my_max_score ||
			(enemy_min_score == my_max_score && average_scaled_score > max_average_scaled_score) ) {
			my_max_score = enemy_min_score;
			max_score_it = my_it;
			max_average_scaled_score = average_scaled_score;
		}
	}

	if (max_score_it != my_move_plans.end())
		best_move_plan = *max_score_it;

	state.bug << "Evaluation finished. Best score: " << my_max_score << endl;
}


// Evaluate a move plan and get the score
int BattleResolver::GetPlanScore(MovePlan& my_plan, MovePlan& enemy_plan) {
	vector<BattleResult> my_battle_res(num_my_ants, BattleResult());
	vector<BattleResult> enemy_battle_res(num_enemy_ants, BattleResult());

	for (int i = 0; i < num_my_ants; i++) {
		for (int j = 0; j < num_enemy_ants; j++) {
			int row_offset = abs(my_plan.new_locations[i].row - enemy_plan.new_locations[j].row);
			row_offset = min(row_offset, state.rows - row_offset);
			int col_offset = abs(my_plan.new_locations[i].col - enemy_plan.new_locations[j].col);
			col_offset = min(col_offset, state.cols - col_offset);
			LocationOffset offset(row_offset, col_offset);

			if (attack_offsets.find(offset) != attack_offsets.end()) {
				my_battle_res[i].num_attackers++;
				my_battle_res[i].attackers.push_back(j);

				enemy_battle_res[j].num_attackers++;
				enemy_battle_res[j].attackers.push_back(i);
			}
			else if (defend_offsets.find(offset) != defend_offsets.end()) {
				my_battle_res[i].num_defenders++;
				my_battle_res[i].defenders.push_back(j);

				enemy_battle_res[j].num_defenders++;
				enemy_battle_res[j].defenders.push_back(i);
			}
			else if (backup_offsets.find(offset) != backup_offsets.end()) {
				my_battle_res[i].num_backups++;
				my_battle_res[i].backups.push_back(j);

				enemy_battle_res[j].num_backups++;
				enemy_battle_res[j].backups.push_back(i);
			}
		}
	}

	// Calculate the score 
	int score = 0;
	score += GetKillingScore(my_battle_res, enemy_battle_res);
	score += GetDefenseScore(my_battle_res, enemy_battle_res);
	//score += GetBackupScore(my_battle_res, enemy_battle_res);
	
	// Add water penalties and food bonuses
	for (int i = 0; i < num_my_ants; i++) {
		Location my_loc = my_plan.new_locations[i];
		score -= state.water_blocking_penalties[my_loc.row][my_loc.col];
		if (!my_ants[i]->IsHillDefender())
			score += state.tile_bonus_values[my_loc.row][my_loc.col];
	}

	for (int e = 0; e < num_enemy_ants; e++) {
		Location enemy_loc = enemy_plan.new_locations[e];
		score += state.water_blocking_penalties[enemy_loc.row][enemy_loc.col];
	}

	return score;
}


// Check killers and killees and return a score
int BattleResolver::GetKillingScore(vector<BattleResult>& my_battle_res, vector<BattleResult>& enemy_battle_res) {
	set<int> my_dead_ants;
	set<int> enemy_dead_ants;
	int my_attack_points = 0;
	for (int i = 0; i < num_my_ants; i++) {
		my_attack_points += my_battle_res[i].num_attackers;
		for (int a = 0; a < my_battle_res[i].num_attackers; a++) {
			int enemy_index = my_battle_res[i].attackers[a];
			if (my_battle_res[i].num_attackers < enemy_battle_res[enemy_index].num_attackers) {
				enemy_dead_ants.insert(enemy_index);
			}
			else if (my_battle_res[i].num_attackers > enemy_battle_res[enemy_index].num_attackers) {
				my_dead_ants.insert(i);
				//score -= KILL_SCORE; // * (my_battle_res[i].num_attackers - enemy_battle_res[enemy_index].num_attackers);
			}
			else {
				my_dead_ants.insert(i);
				enemy_dead_ants.insert(enemy_index);
			}
		}
	}

	// Calculate killing scores
	int enemy_deaths = enemy_dead_ants.size();
	int my_deaths = my_dead_ants.size();

	// Discourage mutual killing when there are no hill defender or attacker
	int mutual_kill_rate = encourage_mutual_kill ? KILL_SCORE : -KILL_SCORE; /*encourage_mutual_kill ? MUTUAL_KILL_SCORE : -MUTUAL_KILL_SCORE;*/
	if (enemy_deaths >= my_deaths) {
		return KILL_SCORE * (enemy_deaths - my_deaths) + mutual_kill_rate * my_deaths + my_attack_points * KILL_SCORE / 20;
	}
	else {
		return KILL_SCORE * (enemy_deaths - my_deaths) - KILL_SCORE * my_deaths + my_attack_points * KILL_SCORE / 20; //+ mutual_kill_rate * enemy_deaths;
	}
}


// Get my defense score
int BattleResolver::GetDefenseScore(vector<BattleResult>& my_battle_res, vector<BattleResult>& enemy_battle_res) {
	set<int> my_potential_deaths;
	set<int> enemy_potential_deaths;
	int my_defend_points = 0;
	for (int i = 0; i < num_my_ants; i++) {
		my_defend_points += my_battle_res[i].num_defenders;

		for (int d = 0; d < my_battle_res[i].num_defenders; d++) {
			int enemy_index = my_battle_res[i].defenders[d];
			if (my_battle_res[i].num_defenders < enemy_battle_res[enemy_index].num_defenders ) {
				enemy_potential_deaths.insert(i);
				//score += DEFEND_SCORE * (enemy_battle_res[enemy_index].num_defenders - my_battle_res[i].num_defenders);
			}
			else if (my_battle_res[i].num_defenders > enemy_battle_res[enemy_index].num_defenders) {
				my_potential_deaths.insert(enemy_index);
				//score -= DEFEND_SCORE * (my_battle_res[i].num_defenders - enemy_battle_res[enemy_index].num_defenders);
			}
			else {
				enemy_potential_deaths.insert(i);
				my_potential_deaths.insert(enemy_index);
			}
		}
	}

	//return DEFEND_SCORE * (my_defend_winners.size() - enemy_defend_winners.size());

	int enemy_deaths = enemy_potential_deaths.size();
	int my_deaths = my_potential_deaths.size();
	int mutual_defend_rate = encourage_mutual_kill ? DEFEND_SCORE : -DEFEND_SCORE;
	if (enemy_deaths >= my_deaths) {
		return DEFEND_SCORE * (enemy_deaths - my_deaths) + mutual_defend_rate * my_deaths 
													+ my_defend_points * DEFEND_SCORE / 10;
	}
	else {
		return DEFEND_SCORE * (enemy_deaths - my_deaths) - DEFEND_SCORE * my_deaths + 
													+ my_defend_points * DEFEND_SCORE / 10; //+ mutual_defend_rate * enemy_deaths;
	}
}


// Evaluate my backup score
int BattleResolver::GetBackupScore(vector<BattleResult>& my_battle_res, vector<BattleResult>& enemy_battle_res) {
	set<int> my_backup_winners;
	set<int> enemy_backup_winners;
	for (int i = 0; i < num_my_ants; i++) {
		for (int b = 0; b < my_battle_res[i].num_backups; b++) {
			int enemy_index = my_battle_res[i].backups[b];
			if (my_battle_res[i].num_backups < enemy_battle_res[enemy_index].num_backups ) {
				my_backup_winners.insert(i);
			}
			else if (my_battle_res[i].num_backups > enemy_battle_res[enemy_index].num_backups) {
				enemy_backup_winners.insert(enemy_index);
			}
			else {
				if (num_my_ants == 1 && num_enemy_ants == 1) {
					if (enemy_ants[enemy_index]->IsHillAttacker()) {
						my_backup_winners.insert(i);
					}
					else {
						enemy_backup_winners.insert(enemy_index);
					}
				}
			}
		}
	}

	return BACKUP_SCORE * (my_backup_winners.size() - enemy_backup_winners.size());
}




// Generate all move plans for my ants or enemy ants
void BattleResolver::GenerateMovePlans(vector<MovePlan>& move_plans, MovePlan current_plan,
										vector<Location>& ants, int pos, bool check_moved) {
	if (pos == ants.size()) {
		move_plans.push_back(current_plan);
		return;
	}

	if (move_plans.size() > MAX_PLAN_LIMIT) {
		return;
	}

	MovePlan new_plan = current_plan;
	int new_pos = pos + 1;

	if ((current_plan.new_loc_map.find(ants[pos]) == current_plan.new_loc_map.end())) {
		new_plan.directions.push_back(MOVE_STAY);
		new_plan.new_locations.push_back(ants[pos]);
		new_plan.new_loc_map[ants[pos]] = ants[pos];
		GenerateMovePlans(move_plans, new_plan, ants, new_pos, check_moved);
	}

	if (check_moved && my_ants[pos]->HasMoved())
		return;

	map<Location, Location>::iterator it;
	for (int d = 0; d < TDIRECTIONS; d++) {
		Location loc = state.getLocation(ants[pos], d);
		if ( (check_moved && GetNumEnemyFighters(loc) == 0) || (!check_moved && GetNumMyFighters(loc) == 0)) {
			continue;
		}

		if (current_plan.new_loc_map.find(loc) != current_plan.new_loc_map.end())
			continue;

		if ( (it = current_plan.new_loc_map.find(ants[pos])) != current_plan.new_loc_map.end() 
				&& loc == it->second) {
			continue;
		}

		// The ant can only move to a place that is either not water, or is occupied by one of the ants in the battle
		// NOTE: the ant should not move to a place with an ant NOT involved in the battle
		if ((!state.IsBlocked(loc) || my_ant_loc_set.find(loc) != my_ant_loc_set.end() || enemy_ant_loc_set.find(loc) != enemy_ant_loc_set.end())) {
				MovePlan plan = current_plan;
				plan.directions.push_back(d);
				plan.new_locations.push_back(loc);
				plan.new_loc_map[loc] = ants[pos];
				GenerateMovePlans(move_plans, plan, ants, new_pos, check_moved);
		}
	}
}


int BattleResolver::GetNumMyFighters(Location loc) {
	return my_attack_area_map[loc].size() + 
		my_defend_area_map[loc].size() + my_backup_area_map[loc].size();
}

int BattleResolver::GetNumEnemyFighters(Location loc) {
	return enemy_attack_area_map[loc].size() + 
		enemy_defend_area_map[loc].size() + enemy_backup_area_map[loc].size();
}


//
MovePlan& BattleResolver::GetBestMovePlan() {
	return best_move_plan;
}



