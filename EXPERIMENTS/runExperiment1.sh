#!/usr/bin/env sh

# EXPERIMENT 1 - VARIETY OF OPPONENTS
# EXPERIMENT SETUP
VERSION=1
TURNS=1000
ROUNDS=10
MAP='../tools/maps/cell_maze/cell_maze_p02_18.map'
PLAYER_SEED=42
END_WAIT=0.25
OURBOT='../MyBot.py'

echo "round,our_score,their_score,our_status,their_status,our_turns,their_turns"

# vs. HunterBot.py
opponent='../tools/sample_bots/python/HunterBot.py'
echo "vs. HunterBot.py"
../tools/playgame.py --player_seed=$PLAYER_SEED --verbose --end_wait=$END_WAIT --turns $TURNS --rounds $ROUNDS --map_file $MAP "$@" "python $OURBOT" "python $opponent" | awk 'BEGIN{OFS=",";} /#/{out=$4;} /^score/{out=out $2 OFS $3; next} /^status/{out=out OFS $2 OFS $3; next} /playerturns/{out=out OFS $2 OFS $3; print out; out=""} END{if (out) print out;}'

####### ADD THE REST BELOW!
# vs. HunterBot.py
#opponent='../tools/sample_bots/python/HunterBot.py'
#echo "vs. HunterBot.py"
#../tools/playgame.py --player_seed=$PLAYER_SEED --verbose --end_wait=$END_WAIT --turns $TURNS --rounds $ROUNDS --map_file $MAP "$@" "python $opponent" "python $OURBOT" | awk 'BEGIN{OFS=",";} /#/{out=$4;} /^score/{out=out $2 OFS $3; next} /^status/{out=out OFS $2 OFS $3; next} /playerturns/{out=out OFS $2 OFS $3; print out; out=""} END{if (out) print out;}'
