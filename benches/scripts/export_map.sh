#!/usr/bin/env bash

# Hopf
./Adams export_map C2 S0 80 | tee -a C2__S0_export.out
./Adams export_map Ceta S0 80 | tee -a Ceta__S0_export.out
./Adams export_map Cnu S0 80 | tee -a Cnu__S0_export.out
./Adams export_map Csigma S0 80 | tee -a Csigma__S0_export.out
