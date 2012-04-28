#!/usr/bin/env sh

for n in 1 2 3
do
    echo "VERSION $n"
    echo "Running experiment 1..."
    ./runExperiment1.sh "$n" > results/v$n-e1.csv
    
    echo "Running experiment 2..."
    ./runExperiment2.sh "$n" > results/v$n-e2.csv
done