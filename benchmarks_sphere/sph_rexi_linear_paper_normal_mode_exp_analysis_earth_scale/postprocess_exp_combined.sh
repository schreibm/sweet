#! /bin/bash


BASEDIR=`pwd`

for i in script_*; do 
	test -d "$i" || continue

	echo "******************************************************"
	echo "* PROCESSING $i"
	echo "******************************************************"
	cd "$i"

	../normal_modes_compute_exp.py ./output_normal_modes_physical_t???????????.????????.csv || exit
	cd "$BASEDIR"
done


FILE="output_normal_modes_physical_t00000000400.00000000.csv_evalues_complex.csv"

./normal_modes_plot_and_analyse_combined.py \
	a_output_combined_fsphere1.pdf	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_cn_tso2_tsob1_C000400_T001_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_erk_tso1_tsob1_C000400_T001_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_erk_tso2_tsob1_C000400_T001_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_erk_tso4_tsob1_C000400_T001_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00000016_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00000032_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00000064_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00000128_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00000256_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00000512_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00001024_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00002048_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00004096_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00008192_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f0.000145842_a6371220_u0_U0_fsph1_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00016384_h0.15_nrm1_hlf1_bf0_ext02_M0016


./normal_modes_plot_and_analyse_combined.py \
	a_output_combined_fsphere0.pdf	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_cn_tso2_tsob1_C000400_T001_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_erk_tso1_tsob1_C000400_T001_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_erk_tso2_tsob1_C000400_T001_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_erk_tso4_tsob1_C000400_T001_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00000016_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00000032_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00000064_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00000128_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00000256_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00000512_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00001024_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00002048_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00004096_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00008192_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE	\
	script_g1_h100000_f7.2921e-05_a6371220_u0_U0_fsph0_tsm_l_rexi_tso0_tsob1_C000400_T001_REXITER_m00016384_h0.15_nrm1_hlf1_bf0_ext02_M0016/$FILE


