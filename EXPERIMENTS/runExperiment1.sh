#!/usr/bin/env sh

# EXPERIMENT 1 - VARIETY OF OPPONENTS
# EXPERIMENT SETUP
TURNS=750
ROUNDS=10
MAP='../tools/maps/cell_maze/cell_maze_p02_20.map'
PLAYER_SEED=42
END_WAIT=0.25
TURNTIME=1000
OURBOT="python ../MyBot.py"

echo "round,our_score,their_score,our_status,their_status,our_turns,their_turns,our_ants,their_ants,in_hive?"


# xathis - 1        java -cp \"opponents/xathis\" MyBot
# rossxwest - 115   java -jar opponents/rossxwest/MyBot.jar
# utoxin - 726      python opponents/utoxin/MyBot.py3
# gakman - 941      python opponents/gakman/MyBot.pypy
# Ficik - 948       java -jar opponents/ficik/dist/MyBot.jar
# wraithan - 1553   python opponents/wraithan/MyBot.pypy
# GreedyBot - 2564  python ../tools/sample_bots/python/GreedyBot.py
opponents=("python ../tools/sample_bots/python/GreedyBot.py"
            "python opponents/wraithan/MyBot.pypy"
            "java -jar opponents/ficik/dist/MyBot.jar"  
            "python opponents/utoxin/MyBot.py3" 
            "java -jar opponents/rossxwest/MyBot.jar")

# VERSION ONE
OURBOT="python our_bot/v$1/MyBot.py"
for opponent in "${opponents[@]}"
do
    echo "vs. $opponent"
    ../tools/playgame.py --player_seed=$PLAYER_SEED --verbose --turntime=$TURNTIME --end_wait=$END_WAIT --turns $TURNS --rounds $ROUNDS --map_file $MAP "$OURBOT" "$opponent" | awk 'BEGIN{OFS=",";} /#/{out=$4;} /^turn.*stats/{ants=substr($4,2,length($4)-2);next} /^score/{out=out $2 OFS $3; next} /^status/{out=out OFS $2 OFS $3; next} /playerturns/{out=out OFS $2 OFS $3 OFS ants; print out; out=""} END{if (out) print out;}'
done