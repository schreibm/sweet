/*
 * SWE_Sphere_TS_ln_settls.hpp
 *
 *  Created on: 24 Sep 2019
 *      Author: Martin Schreiber <SchreiberX@gmail.com>
 *
 *  Based on plane code
 */

#ifndef SRC_PROGRAMS_SWE_SPHERE_REXI_SWE_SPHERE_TS_LN_SETTLS_HPP_
#define SRC_PROGRAMS_SWE_SPHERE_REXI_SWE_SPHERE_TS_LN_SETTLS_HPP_

#include <limits>
#include <string>
#include <sweet/SimulationVariables.hpp>
#include <sweet/sphere/SphereData_Physical.hpp>
#include <sweet/sphere/SphereData_Spectral.hpp>
#include <sweet/sphere/SphereOperators_SphereData.hpp>
#include <sweet/sphere/SphereOperators_Sampler_SphereDataPhysical.hpp>
#include <sweet/sphere/SphereTimestepping_SemiLagrangian.hpp>
#include <sweet/sphere/SphereTimestepping_ExplicitRK.hpp>

#include "SWE_Sphere_TS_interface.hpp"
#include "SWE_Sphere_TS_l_irk.hpp"
#include "SWE_Sphere_TS_lg_irk.hpp"
#include "SWE_Sphere_TS_l_erk.hpp"
#include "SWE_Sphere_TS_lg_erk.hpp"
#include "SWE_Sphere_TS_l_rexi.hpp"



class SWE_Sphere_TS_ln_settls	: public SWE_Sphere_TS_interface
{
public:
	static bool implements_timestepping_method(const std::string &i_timestepping_method)
	{
		/*
		 * Should contain _exp and _settls
		 */
		return (
			(i_timestepping_method.find("_settls") != std::string::npos)
			&&
			!(i_timestepping_method.find("_exp") != std::string::npos)
		);

		return false;
	}

	std::string string_id_storage;

	std::string string_id()
	{
		return string_id_storage;
	}


	void setup_auto();


private:
	SimulationVariables &simVars;
	SphereOperators_SphereData &op;

public:
	enum LinearTreatment_enum {
		LINEAR_IGNORE,
		LINEAR_IMPLICIT,
		LINEAR_EXPONENTIAL,
	};

	enum LinearCoriolisTreatment_enum {
		CORIOLIS_IGNORE,
		CORIOLIS_LINEAR,
		CORIOLIS_NONLINEAR,
		CORIOLIS_SEMILAGRANGIAN,
	};

	enum NLAdvectionTreatment_enum {
		NL_ADV_IGNORE,
		NL_ADV_SEMILAGRANGIAN,
	};

	enum NLDivergenceTreatment_enum{
		NL_DIV_IGNORE,
		NL_DIV_NONLINEAR,
	};

private:
	LinearTreatment_enum linear_treatment;
	LinearCoriolisTreatment_enum coriolis_treatment;
	NLAdvectionTreatment_enum nonlinear_advection_treatment;
	NLDivergenceTreatment_enum nonlinear_divergence_treatment;
	int timestepping_order;
	bool original_linear_operator_sl_treatment;

	SphereTimestepping_SemiLagrangian semiLagrangian;
	SphereOperators_Sampler_SphereDataPhysical sphereSampler;

	SphereData_Spectral U_phi_pert_prev, U_vrt_prev, U_div_prev;

	SWE_Sphere_TS_l_erk* swe_sphere_ts_l_erk;
	SWE_Sphere_TS_lg_erk* swe_sphere_ts_lg_erk;
	SWE_Sphere_TS_l_irk* swe_sphere_ts_l_irk;
	SWE_Sphere_TS_lg_irk* swe_sphere_ts_lg_irk;
	SWE_Sphere_TS_l_rexi *swe_sphere_ts_l_rexi;



public:
	SWE_Sphere_TS_ln_settls(
			SimulationVariables &i_simVars,
			SphereOperators_SphereData &i_op
		);


	void setup(
			int i_timestepping_order,
			LinearTreatment_enum i_linear_treatment,
			LinearCoriolisTreatment_enum i_coriolis_treatment,// = SWE_Sphere_TS_ln_settls::CORIOLIS_LINEAR,		// "ignore", "linear", "nonlinear", "semi-lagrangian"
			NLAdvectionTreatment_enum i_nonlinear_advection_treatment,
			NLDivergenceTreatment_enum i_nonlinear_divergence_treatment,// = SWE_Sphere_TS_ln_settls::NL_DIV_NONLINEAR,	// "ignore", "nonlinear"
			bool original_linear_operator_sl_treatment// = true
	);


	void run_timestep_pert(
			SphereData_Spectral &io_phi,	///< prognostic variables
			SphereData_Spectral &io_vort,	///< prognostic variables
			SphereData_Spectral &io_div,	///< prognostic variables

			double i_dt = 0,		///< if this value is not equal to 0, use this time step size instead of computing one
			double i_simulation_timestamp = -1
	);


	void run_timestep_nonpert(
			SphereData_Spectral &io_phi,	///< prognostic variables
			SphereData_Spectral &io_vort,	///< prognostic variables
			SphereData_Spectral &io_div,	///< prognostic variables

			double i_dt = 0,		///< if this value is not equal to 0, use this time step size instead of computing one
			double i_simulation_timestamp = -1
	);


	void run_timestep_1st_order(
			SphereData_Spectral &io_phi,	///< prognostic variables
			SphereData_Spectral &io_vort,	///< prognostic variables
			SphereData_Spectral &io_div,	///< prognostic variables

			double i_dt = 0,		///< if this value is not equal to 0, use this time step size instead of computing one
			double i_simulation_timestamp = -1
	);


	void run_timestep_2nd_order(
			SphereData_Spectral &io_phi,	///< prognostic variables
			SphereData_Spectral &io_vort,	///< prognostic variables
			SphereData_Spectral &io_div,	///< prognostic variables

			double i_dt = 0,		///< if this value is not equal to 0, use this time step size instead of computing one
			double i_simulation_timestamp = -1
	);



	void run_timestep_2nd_order_pert(
			SphereData_Spectral &io_phi_pert,	///< prognostic variables
			SphereData_Spectral &io_vort,		///< prognostic variables
			SphereData_Spectral &io_div,		///< prognostic variables

			double i_dt = 0,		///< if this value is not equal to 0, use this time step size instead of computing one
			double i_simulation_timestamp = -1
	);


	void run_timestep_2nd_order_pert_REAL(
			SphereData_Spectral &io_phi_pert,	///< prognostic variables
			SphereData_Spectral &io_vort,		///< prognostic variables
			SphereData_Spectral &io_div,		///< prognostic variables

			double i_dt = 0,		///< if this value is not equal to 0, use this time step size instead of computing one
			double i_simulation_timestamp = -1
	);



	virtual ~SWE_Sphere_TS_ln_settls();
};

#endif /* SRC_PROGRAMS_SWE_SPHERE_REXI_SWE_SPHERE_TS_L_CN_NA_SL_ND_SETTLS_HPP_ */
