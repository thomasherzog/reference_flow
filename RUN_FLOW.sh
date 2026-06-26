#!/bin/bash

bender update

# Verilator Setup

cd verilator

./run_verilator.sh --flist
./run_verilator.sh --build

cd ..

# Synthesis via yosys

cd yosys

./run_synthesis.sh --flist
./run_synthesis.sh --synth

cd ..

# OpenRoad

#cd openroad



#cd ..
