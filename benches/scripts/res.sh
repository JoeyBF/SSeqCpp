#!/usr/bin/env bash

degree=$1

echo "--- Computing resolutions ---"

# Rings
./Adams res S0 $degree | tee -a S0_res.out

# Hopf
./Adams res C2 $degree | tee -a C2_res.out
./Adams res Ceta $degree | tee -a Ceta_res.out
./Adams res Cnu $degree | tee -a Cnu_res.out
./Adams res Csigma $degree | tee -a Csigma_res.out
