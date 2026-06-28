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

cd openroad

openroad -gui           # source 01 bis 08

cd ..

# KLayout : DRC

cd ./klayout

./def2gds-croc 

./run_drc-croc          # if errors, fix by hand...

cd ..

# Calibre : LVS

cd calibre

cd lvs

./verilog2spice ../../openroad/out/croc_lvs.v croc_chip.spice

cd ..

./start-calibre

cd ..