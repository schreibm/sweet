/*
 * PDESWESphereTS_l_irk_na_sl_nr_settls_vd_only.hpp
 *
 *  Created on: 01 Apr 2020
 * Author: Martin SCHREIBER <schreiberx@gmail.com>
 *
 *  Based on plane code
 */

#ifndef SRC_PROGRAMS_SWE_SPHERE_REXI_SWE_SPHERE_TS_L_IRK_NA_SL_NR_SETTLS_VD_ONLY_HPP_
#define SRC_PROGRAMS_SWE_SPHERE_REXI_SWE_SPHERE_TS_L_IRK_NA_SL_NR_SETTLS_VD_ONLY_HPP_


#include <sweet/core/shacks/ShackDictionary.hpp>
#include <sweet/core/sphere/SphereData_Physical.hpp>
#include <sweet/core/sphere/SphereData_Spectral.hpp>
#include <sweet/core/sphere/SphereOperators.hpp>
#include <sweet/core/sphere/SphereOperators_Sampler_SphereDataPhysical.hpp>
#include <sweet/core/time/TimesteppingSemiLagrangianSphereData.hpp>
#include <sweet/core/time/TimesteppingExplicitRKSphereData.hpp>

#include "PDESWESphereTS_BaseInterface.hpp"
#include "PDESWESphereTS_l_irk.hpp"
#include "PDESWESphereTS_ln_erk_split_vd.hpp"



class PDESWESphereTS_l_irk_na_sl_nr_settls_vd_only	: public PDESWESphereTS_BaseInterface
{

public:
	bool setup_auto(
			const std::string &i_timestepping_method,
			sweet::SphereOperators *io_ops
		);

	bool setup_main(
			sweet::SphereOperators *io_ops,
			int i_timestepping_order
		);

public:
	bool implementsTimesteppingMethod(const std::string &i_timestepping_method);
	std::string getIDString();

private:
	sweet::TimesteppingSemiLagrangianSphereData semiLagrangian;

	sweet::SphereData_Spectral U_phi_prev, U_vrt_prev, U_div_prev;

	PDESWESphereTS_ln_erk_split_vd swe_sphere_ts_ln_erk_split_vd__l_erk_1st_order;
	PDESWESphereTS_l_irk swe_sphere_ts_l_irk;



public:
	bool shackRegistration(
			sweet::ShackDictionary *io_shackDict
	)
	{
		PDESWESphereTS_BaseInterface::shackRegistration(io_shackDict);

		swe_sphere_ts_ln_erk_split_vd__l_erk_1st_order.shackRegistration(io_shackDict);
		swe_sphere_ts_l_irk.shackRegistration(io_shackDict);
		return true;
	}


public:
	PDESWESphereTS_l_irk_na_sl_nr_settls_vd_only();


	void runTimestep(
			sweet::SphereData_Spectral &io_phi,	///< prognostic variables
			sweet::SphereData_Spectral &io_vort,	///< prognostic variables
			sweet::SphereData_Spectral &io_div,	///< prognostic variables

			double i_dt = 0,
			double i_simulation_timestamp = -1
		);

	void run_timestep_2nd_order(
			sweet::SphereData_Spectral &io_phi_pert,	///< prognostic variables
			sweet::SphereData_Spectral &io_vort,		///< prognostic variables
			sweet::SphereData_Spectral &io_div,		///< prognostic variables

			double i_dt = 0,
			double i_simulation_timestamp = -1
		);

#if (SWEET_PARAREAL && SWEET_PARAREAL_SPHERE) || (SWEET_XBRAID && SWEET_XBRAID_SPHERE)
	void set_previous_solution(
				sweet::SphereData_Spectral &i_phi_prev,
				sweet::SphereData_Spectral &i_vrt_prev,
				sweet::SphereData_Spectral &i_div_prev
	) override
	{
		if (shackDict.misc.verbosity > 5)
			std::cout << "set_previous_solution()" << std::endl;
		U_phi_prev = i_phi_prev;
		U_vrt_prev = i_vrt_prev;
		U_div_prev = i_div_prev;
	}
#endif

	virtual ~PDESWESphereTS_l_irk_na_sl_nr_settls_vd_only();
};

#endif
