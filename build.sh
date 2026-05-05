#!/bin/bash

if [ ! -d "build" ]; then
    echo "Creating build directory ..."
    mkdir build
fi

cd build

# Создаём папки genomes и screenshots внутри build
if [ ! -d "genomes" ]; then
    echo "Creating genomes directory ..."
    mkdir genomes
fi

if [ ! -d "screenshots" ]; then
    echo "Creating screenshots directory ..."
    mkdir screenshots
fi

cmake ..
make -j3
