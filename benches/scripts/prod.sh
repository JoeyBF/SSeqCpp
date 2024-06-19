#!/usr/bin/env bash

# Rings
./Adams prod S0 80 | tee -a S0_prod.out

# Hopf
./Adams prod_mod C2 S0 80 | tee -a C2_prod.out
./Adams prod_mod Ceta S0 80 | tee -a Ceta_prod.out
./Adams prod_mod Cnu S0 80 | tee -a Cnu_prod.out
./Adams prod_mod Csigma S0 80 | tee -a Csigma_prod.out
