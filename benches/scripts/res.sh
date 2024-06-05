#!/usr/bin/env bash

# Rings
./Adams res S0 100 | tee -a S0_res.out
./Adams res tmf 100 | tee -a tmf_res.out

# Hopf
./Adams res C2 100 | tee -a C2_res.out
./Adams res Ceta 100 | tee -a Ceta_res.out
./Adams res Cnu 100 | tee -a Cnu_res.out
./Adams res Csigma 100 | tee -a Csigma_res.out

# sigmasq
./Adams res Csigmasq 100 | tee -a Csigmasq_res.out
./Adams res C2h4 100 | tee -a C2h4_res.out
./Adams res DC2h4 100 | tee -a DC2h4_res.out
./Adams res CW_sigmasq_2_sigmasq 100 | tee -a CW_sigmasq_2_sigmasq_res.out

# theta4
./Adams res Ctheta4 100 | tee -a Ctheta4_res.out
./Adams res C2h5 100 | tee -a C2h5_res.out
./Adams res DC2h5 100 | tee -a DC2h5_res.out
./Adams res CW_theta4_2_theta4 100 | tee -a CW_theta4_2_theta4_res.out

# theta5
./Adams res Ctheta5 100 | tee -a Ctheta5_res.out
./Adams res C2h6 100 | tee -a C2h6_res.out
./Adams res DC2h6 100 | tee -a DC2h6_res.out
./Adams res CW_theta5_2_theta5 100 | tee -a CW_theta5_2_theta5_res.out
./Adams res CW_2_theta5_2_Eq_eta_theta5 100 | tee -a CW_2_theta5_2_Eq_eta_theta5_res.out
