#!/usr/bin/env bash

# Hopf
./Adams map_res C2 S0 80 | tee -a C2__S0_res.out
./Adams map_res Ceta S0 80 | tee -a Ceta__S0_res.out
./Adams map_res Cnu S0 80 | tee -a Cnu__S0_res.out
./Adams map_res Csigma S0 80 | tee -a Csigma__S0_res.out
