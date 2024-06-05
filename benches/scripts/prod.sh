#!/usr/bin/env bash

# Rings
./Adams prod S0 100 | tee -a S0_prod.out
./Adams prod tmf 100 | tee -a tmf_prod.out

# Hopf
./Adams prod_mod C2 S0 100 | tee -a C2_prod.out
./Adams prod_mod Ceta S0 100 | tee -a Ceta_prod.out
./Adams prod_mod Cnu S0 100 | tee -a Cnu_prod.out
./Adams prod_mod Csigma S0 100 | tee -a Csigma_prod.out

# sigmasq
./Adams prod_mod Csigmasq S0 100 | tee -a Csigmasq_prod.out
./Adams prod_mod C2h4 S0 100 | tee -a C2h4_prod.out
./Adams prod_mod DC2h4 S0 100 | tee -a DC2h4_prod.out
./Adams prod_mod CW_sigmasq_2_sigmasq S0 100 | tee -a CW_sigmasq_2_sigmasq_prod.out

# theta4
./Adams prod_mod Ctheta4 S0 100 | tee -a Ctheta4_prod.out
./Adams prod_mod C2h5 S0 100 | tee -a C2h5_prod.out
./Adams prod_mod DC2h5 S0 100 | tee -a DC2h5_prod.out
./Adams prod_mod CW_theta4_2_theta4 S0 100 | tee -a CW_theta4_2_theta4_prod.out

# theta5
./Adams prod_mod Ctheta5 S0 100 | tee -a Ctheta5_prod.out
./Adams prod_mod C2h6 S0 100 | tee -a C2h6_prod.out
./Adams prod_mod DC2h6 S0 100 | tee -a DC2h6_prod.out
./Adams prod_mod CW_theta5_2_theta5 S0 100 | tee -a CW_theta5_2_theta5_prod.out
./Adams prod_mod CW_2_theta5_2_Eq_eta_theta5 S0 100 | tee -a CW_2_theta5_2_Eq_eta_theta5_prod.out
