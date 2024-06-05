#!/usr/bin/env bash

# Rings
./Adams map_res S0 tmf 100 | tee -a S0__tmf_res.out

# Hopf
./Adams map_res C2 S0 100 | tee -a C2__S0_res.out
./Adams map_res Ceta S0 100 | tee -a Ceta__S0_res.out
./Adams map_res Cnu S0 100 | tee -a Cnu__S0_res.out
./Adams map_res Csigma S0 100 | tee -a Csigma__S0_res.out
