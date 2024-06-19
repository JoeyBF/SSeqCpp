#!/usr/bin/env bash

# Rings
./Adams export S0 80 | tee -a S0_export.out

# Hopf
./Adams export_mod C2 S0 80 | tee -a C2_export.out
./Adams export_mod Ceta S0 80 | tee -a Ceta_export.out
./Adams export_mod Cnu S0 80 | tee -a Cnu_export.out
./Adams export_mod Csigma S0 80 | tee -a Csigma_export.out
