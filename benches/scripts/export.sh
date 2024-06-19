#!/usr/bin/env bash

degree=$1

# Rings
./Adams export S0 $degree | tee -a S0_export.out

# Hopf
./Adams export_mod C2 S0 $degree | tee -a C2_export.out
./Adams export_mod Ceta S0 $degree | tee -a Ceta_export.out
./Adams export_mod Cnu S0 $degree | tee -a Cnu_export.out
./Adams export_mod Csigma S0 $degree | tee -a Csigma_export.out
