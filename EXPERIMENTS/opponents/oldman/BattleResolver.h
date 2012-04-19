//------------------------ BattleResolver.h -------------------------------------

#ifndef BATTLE_RESOLVER_H
#define BATTLE_RESOLVER_H

#include "State.h"
#include "AntInfo.h"
using namespace std;

struct MovePlan {
	vector<int> directions;
	vector<Location> new_locations;
	map<Location, Location> new_loc_map;		// new location, original location 	
};

struct BattleResult {
	int num_attackers;
	int num_defenders;
	int num_backups;
	vector<int>	attackers;
	vector<int> defenders;
	vector<int> backups;

	BattleResult() {
		num_attackers = 0;
		num_defenders = 0;
		num_backups = 0;
	}
};



//
class BattleResolver {
public:
	/*BattleResolver(State& s, vector<Location> my_ants, vector<bool> has_moved, 
					vector<bool> is_hill_defender, vector<bool> is_hill_attacker,
					vector<Location> enemy_ants, Location& enemy_loc);*/

	BattleResolver(State& s, vector<AntInfo*>& my_ants, vector<AntInfo*>& enemy_ants, Location& enemy_loc);

	void		ResolveBattle();
	MovePlan&	GetBestMovePlan();

	static set<LocationOffset> attack_offsets;
	static set<LocationOffset> defend_offsets;
	static set<LocationOffset> backup_offsets;
	static map<Location, set<Location> > my_attack_area_map;
	static map<Location, set<Location> > my_defend_area_map;
	static map<Location, set<Location> > my_backup_area_map;
	static map<Location, set<Location> > enemy_attack_area_map;
	static map<Location, set<Location> > enemy_defend_area_map;
	static map<Location, set<Location> > enemy_backup_area_map;
	
	static const int KILL_SCORE = 1000000;
	static const int MUTUAL_KILL_SCORE = 50000;
	static const int DEFEND_SCORE = 1000;
	static const int MUTUAL_DEFEND_SCORE = 500;
	static const int BACKUP_SCORE = 100;
	static const int MUTUAL_BACKUP_SCORE = 50;
	static const int OUT_OF_BATTLE_SCORE = 50;

	static const int MAX_PLAN_LIMIT = 1500;
	static const int MAX_MIN_MAX_SITUATIONS = 30000;

private:
	State&		state;
	Location	enemy_location;

	vector<AntInfo*>	my_ants;
	vector<Location>	my_ant_locs;
	set<Location>		my_ant_loc_set;
	int					num_my_ants;

	vector<AntInfo*>	enemy_ants;
	vector<Location>	enemy_ant_locs;
	set<Location>		enemy_ant_loc_set;
	int					num_enemy_ants;

	int					num_moved_ants;
	bool				encourage_mutual_kill;
	int					encourage_mutual_kill_level;
	bool				has_defenders;
	//vector<bool>		has_moved;
	//vector<bool>		is_hill_defender;
	//vector<bool>		is_hill_attacker;
	
	MovePlan			best_move_plan;

	void ChooseSimpleBattleStrategy(vector<MovePlan>& my_move_plans);
	void ChooseManyToManyStrategy();
	void ChooseMinMaxBattleStrategy(vector<MovePlan>& my_move_plans, vector<MovePlan>& enemy_move_plans);
	int GetPlanScore(MovePlan& my_plan, MovePlan& enemy_plan);
	int GetKillingScore(vector<BattleResult>& my_battle_res, vector<BattleResult>& enemy_battle_res);
	int GetDefenseScore(vector<BattleResult>& my_battle_res, vector<BattleResult>& enemy_battle_res);
	int GetBackupScore(vector<BattleResult>& my_battle_res, vector<BattleResult>& enemy_battle_res);

	int GetNumMyFighters(Location loc);
	int GetNumEnemyFighters(Location loc);

	void GenerateMovePlans(vector<MovePlan>& move_plans, MovePlan current_plan,
							vector<Location>& ants, int pos, bool check_moved);
	
};

#endif