#!/usr/bin/env bash

# Rings
./Adams export_d2 S0 | tee -a S0_export_d2.out

# Hopf
./Adams export_d2 C2 | tee -a C2_export_d2.out
./Adams export_d2 Ceta | tee -a Ceta_export_d2.out
./Adams export_d2 Cnu | tee -a Cnu_export_d2.out
./Adams export_d2 Csigma | tee -a Csigma_export_d2.out
