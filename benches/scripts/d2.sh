#!/usr/bin/env bash

# Rings
./Adams d2 S0 80 | tee -a S0_d2.out

# Hopf
./Adams d2 C2 80 | tee -a C2_d2.out
./Adams d2 Ceta 80 | tee -a Ceta_d2.out
./Adams d2 Cnu 80 | tee -a Cnu_d2.out
./Adams d2 Csigma 80 | tee -a Csigma_d2.out
