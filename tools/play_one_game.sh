#!/usr/bin/env sh
./playgame.py -e --player_seed 42 --end_wait=0.25 --verbose --log_dir game_logs --turns 1000 --map_file maps/cell_maze/cell_maze_p02_20.map "$@" "python ../MyBot.py" "python sample_bots/python/GreedyBot.py"
