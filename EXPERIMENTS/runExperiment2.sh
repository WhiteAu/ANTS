#!/usr/bin/env sh

# EXPERIMENT 2 - VARIETY OF TERRAINS
# EXPERIMENT SETUP
TURNS=750
ROUNDS=10
OPPONENT='java -jar opponents/ficik/dist/MyBot.jar' 
PLAYER_SEED=42
END_WAIT=0.25
TURNTIME=1000

echo "round,our_score,their_score,our_status,their_status,our_turns,their_turns,our_ants,their_ants,in_hive?"

# Cell Maze - ../tools/maps/cell_maze/cell_maze_p02_01.map
# Cell Maze - ../tools/maps/cell_maze/cell_maze_p02_20.map
# Maze      - ../tools/maps/maze/maze_p02_31.map
# Maze      - ../tools/maps/maze/mase_p02_35.map
# Random    - ../tools/maps/random_walk/random_walk_p02_65.map
# Random    - ../tools/maps/random_walk/random_walk_p02_21.map
maps=("../tools/maps/cell_maze/cell_maze_p02_20.map"
      "../tools/maps/maze/maze_p02_31.map"
      "../tools/maps/random_walk/random_walk_p02_21.map")

OURBOT="python our_bot/v$1/MyBot.py"
for map in "${maps[@]}"
do
    echo "on: $map"
    ../tools/playgame.py --player_seed=$PLAYER_SEED --verbose --turntime=$TURNTIME --end_wait=$END_WAIT --turns $TURNS --rounds $ROUNDS --map_file $map "$OURBOT" "$OPPONENT" | awk 'BEGIN{OFS=",";} /#/{out=$4;} /^turn.*stats/{ants=substr($4,2,length($4)-2);next} /^score/{out=out $2 OFS $3; next} /^status/{out=out OFS $2 OFS $3; next} /playerturns/{out=out OFS $2 OFS $3 OFS ants; print out; out=""} END{if (out) print out;}'
done