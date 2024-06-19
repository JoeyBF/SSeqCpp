#!/usr/bin/env bash

degree=$1

echo "--- Computing products ---"

# Rings
./Adams prod S0 $degree | tee -a S0_prod.out

# Hopf
./Adams prod_mod C2 S0 $degree | tee -a C2_prod.out
./Adams prod_mod Ceta S0 $degree | tee -a Ceta_prod.out
./Adams prod_mod Cnu S0 $degree | tee -a Cnu_prod.out
./Adams prod_mod Csigma S0 $degree | tee -a Csigma_prod.out
