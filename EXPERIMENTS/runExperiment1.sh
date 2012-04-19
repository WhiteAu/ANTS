#!/usr/bin/env sh

# EXPERIMENT 1 - VARIETY OF OPPONENTS
# EXPERIMENT SETUP
TURNS=10
ROUNDS=10
MAP='../tools/maps/cell_maze/cell_maze_p02_11.map'
PLAYER_SEED=42
END_WAIT=0.25
OURBOT='../MyBot.py'

echo "round,our_score,their_score,our_status,their_status,our_turns,their_turns,our_ants,their_ants,in_hive?"


### MAKE THIS ITERATE OVER A LIST OF STRINGS FOR OPONENTS LATER PLS

opponents=("python ../tools/sample_bots/python/HunterBot.py" "java -cp \"opponents/xathis\" MyBot" "./oldman")

for opponent in "${opponents[@]}"
do
    echo "vs. $opponent"
    ../tools/playgame.py --player_seed=$PLAYER_SEED --verbose --end_wait=$END_WAIT --turns $TURNS --rounds $ROUNDS --map_file $MAP "$@" "python $OURBOT" "$opponent" | awk 'BEGIN{OFS=",";} /#/{out=$4;} /^turn.*stats/{ants=substr($4,2,length($4)-2);next} /^score/{out=out $2 OFS $3; next} /^status/{out=out OFS $2 OFS $3; next} /playerturns/{out=out OFS $2 OFS $3 OFS ants; print out; out=""} END{if (out) print out;}'
done