#!/usr/bin/env bash

# Rings
./Adams export_map S0 tmf 100 | tee -a S0__tmf_export.out

# Hopf
./Adams export_map C2 S0 100 | tee -a C2__S0_export.out
./Adams export_map Ceta S0 100 | tee -a Ceta__S0_export.out
./Adams export_map Cnu S0 100 | tee -a Cnu__S0_export.out
./Adams export_map Csigma S0 100 | tee -a Csigma__S0_export.out
