#!/usr/bin/env bash

degree=$1

echo "--- Computing maps ---"

# Hopf
./Adams map_res C2 S0 $degree | tee -a C2__S0_res.out
./Adams map_res Ceta S0 $degree | tee -a Ceta__S0_res.out
./Adams map_res Cnu S0 $degree | tee -a Cnu__S0_res.out
./Adams map_res Csigma S0 $degree | tee -a Csigma__S0_res.out
