#!/usr/bin/env bash

# Rings
./Adams export S0 100 | tee -a S0_export.out
./Adams export tmf 100 | tee -a tmf_export.out

# Hopf
./Adams export_mod C2 S0 100 | tee -a C2_export.out
./Adams export_mod Ceta S0 100 | tee -a Ceta_export.out
./Adams export_mod Cnu S0 100 | tee -a Cnu_export.out
./Adams export_mod Csigma S0 100 | tee -a Csigma_export.out

# sigmasq
./Adams export_mod Csigmasq S0 100 | tee -a Csigmasq_export.out
./Adams export_mod C2h4 S0 100 | tee -a C2h4_export.out
./Adams export_mod DC2h4 S0 100 | tee -a DC2h4_export.out
./Adams export_mod CW_sigmasq_2_sigmasq S0 100 | tee -a CW_sigmasq_2_sigmasq_export.out

# theta4
./Adams export_mod Ctheta4 S0 100 | tee -a Ctheta4_export.out
./Adams export_mod C2h5 S0 100 | tee -a C2h5_export.out
./Adams export_mod DC2h5 S0 100 | tee -a DC2h5_export.out
./Adams export_mod CW_theta4_2_theta4 S0 100 | tee -a CW_theta4_2_theta4_export.out

# theta5
./Adams export_mod Ctheta5 S0 100 | tee -a Ctheta5_export.out
./Adams export_mod C2h6 S0 100 | tee -a C2h6_export.out
./Adams export_mod DC2h6 S0 100 | tee -a DC2h6_export.out
./Adams export_mod CW_theta5_2_theta5 S0 100 | tee -a CW_theta5_2_theta5_export.out
./Adams export_mod CW_2_theta5_2_Eq_eta_theta5 S0 100 | tee -a CW_2_theta5_2_Eq_eta_theta5_export.out
