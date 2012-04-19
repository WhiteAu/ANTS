#!/usr/bin/env sh

# EXPERIMENT 2 - VARIETY OF TERRAINS
# EXPERIMENT SETUP
VERSION=1
TURNS=1000
ROUNDS=10
OPPONENT='../tools/sample_bots/python/HunterBot.py' #we should probably pick someone better...
PLAYER_SEED=42
END_WAIT=0.25
OURBOT='../MyBot.py'

echo "round,our_score,their_score,our_status,their_status,our_turns,their_turns"

# Cell Maze
map='../tools/maps/cell_maze/cell_maze_p02_18.map'
echo "Cell Maze"
../tools/playgame.py --player_seed=$PLAYER_SEED --verbose --end_wait=$END_WAIT --turns $TURNS --rounds $ROUNDS --map_file $map "$@" "python $OURBOT" "python $OPPONENT" | awk 'BEGIN{OFS=",";} /#/{out=$4;} /^score/{out=out $2 OFS $3; next} /^status/{out=out OFS $2 OFS $3; next} /playerturns/{out=out OFS $2 OFS $3; print out; out=""} END{if (out) print out;}'

#### ADD THE REST BELOW
# Cell Maze
#map='../tools/maps/cell_maze/cell_maze_p02_18.map'
#echo "Cell Maze"
#../tools/playgame.py --player_seed=$PLAYER_SEED --verbose --end_wait=$END_WAIT --turns $TURNS --rounds $ROUNDS --map_file $map "$@" "python $OURBOT" "python $OPPONENT" | awk 'BEGIN{OFS=",";} /#/{out=$4;} /^score/{out=out $2 OFS $3; next} /^status/{out=out OFS $2 OFS $3; next} /playerturns/{out=out OFS $2 OFS $3; print out; out=""} END{if (out) print out;}'
