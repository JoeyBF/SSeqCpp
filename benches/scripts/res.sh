#!/usr/bin/env bash

# Rings
./Adams res S0 80 | tee -a S0_res.out

# Hopf
./Adams res C2 80 | tee -a C2_res.out
./Adams res Ceta 80 | tee -a Ceta_res.out
./Adams res Cnu 80 | tee -a Cnu_res.out
./Adams res Csigma 80 | tee -a Csigma_res.out
