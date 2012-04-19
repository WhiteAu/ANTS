#!/usr/bin/env sh

VERSION=$1

echo "Running experiment 1..."
./runExperiment1.sh > results/v$VERSION-e1.csv


echo "Running experiment 2..."
./runExperiment1.sh > results/v$VERSION-e2.csv