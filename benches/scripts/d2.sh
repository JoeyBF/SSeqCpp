#!/usr/bin/env bash

degree=$1

echo "--- Computing d2 ---"

# Rings
./Adams d2 S0 $degree | tee -a S0_d2.out

# Hopf
./Adams d2 C2 $degree | tee -a C2_d2.out
./Adams d2 Ceta $degree | tee -a Ceta_d2.out
./Adams d2 Cnu $degree | tee -a Cnu_d2.out
./Adams d2 Csigma $degree | tee -a Csigma_d2.out
