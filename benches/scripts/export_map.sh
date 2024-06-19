#!/usr/bin/env bash

degree=$1

# Hopf
./Adams export_map C2 S0 $degree | tee -a C2__S0_export.out
./Adams export_map Ceta S0 $degree | tee -a Ceta__S0_export.out
./Adams export_map Cnu S0 $degree | tee -a Cnu__S0_export.out
./Adams export_map Csigma S0 $degree | tee -a Csigma__S0_export.out
