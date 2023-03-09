/*
 * PDESWESphereTS_l_phi0_n_edt.hpp
 *
 *  Created on: 29 May 2017
 *      Author: Martin SCHREIBER <schreiberx@gmail.com>
 */

#ifndef SRC_PROGRAMS_SWE_PLANE_REXI_PDESWESphereTS_l_rexi_n_etdrk_HPP_
#define SRC_PROGRAMS_SWE_PLANE_REXI_PDESWESphereTS_l_rexi_n_etdrk_HPP_

#include <sweet/core/sphere/SphereData_Spectral.hpp>
#include <sweet/core/sphere/SphereOperators.hpp>
#include <limits>
#include <sweet/core/shacks/ShackDictionary.hpp>

#include "PDESWESphereTS_BaseInterface.hpp"
#include "PDESWESphereTS_l_erk_n_erk.hpp"
#include "PDESWESphereTS_l_exp.hpp"


class PDESWESphereTS_l_exp_n_etdrk	: public PDESWESphereTS_BaseInterface
{
public:
	bool shackRegistration(sweet::ShackDictionary *io_shackDict);

	bool setup_auto(sweet::SphereOperators *io_ops);

	bool setup(
			sweet::SphereOperators *io_ops,
			sweet::ShackExpIntegration *i_shackExpIntegration,
			const std::string &i_exp_method,
			int i_timestepping_order,
			int i_timestepping_order2,
			double i_timestep_size,
			bool i_use_rexi_sphere_solver_preallocation
	);

public:
	bool implementsTimesteppingMethod(const std::string &i_timestepping_method);

	std::string getIDString();

private:
	PDESWESphereTS_l_erk_n_erk ts_l_erk_n_erk;

	PDESWESphereTS_l_exp ts_phi0_exp;
	PDESWESphereTS_l_exp ts_phi1_exp;
	PDESWESphereTS_l_exp ts_phi2_exp;

	PDESWESphereTS_l_exp ts_ups0_exp;
	PDESWESphereTS_l_exp ts_ups1_exp;
	PDESWESphereTS_l_exp ts_ups2_exp;
	PDESWESphereTS_l_exp ts_ups3_exp;

private:
	void euler_timestep_update_linear(
			const sweet::SphereData_Spectral &i_h,	///< prognostic variables
			const sweet::SphereData_Spectral &i_u,	///< prognostic variables
			const sweet::SphereData_Spectral &i_v,	///< prognostic variables

			sweet::SphereData_Spectral &o_h_t,	///< time updates
			sweet::SphereData_Spectral &o_u_t,	///< time updates
			sweet::SphereData_Spectral &o_v_t,	///< time updates

			double i_max_timestamp
	);



private:
	void euler_timestep_update_nonlinear(
			const sweet::SphereData_Spectral &i_h,	///< prognostic variables
			const sweet::SphereData_Spectral &i_u,	///< prognostic variables
			const sweet::SphereData_Spectral &i_v,	///< prognostic variables

			sweet::SphereData_Spectral &o_h_t,	///< time updates
			sweet::SphereData_Spectral &o_u_t,	///< time updates
			sweet::SphereData_Spectral &o_v_t,	///< time updates

			double i_max_timestamp
	);


public:
	PDESWESphereTS_l_exp_n_etdrk();

	void runTimestep(
			sweet::SphereData_Spectral &io_phi_pert,	///< prognostic variables
			sweet::SphereData_Spectral &io_vrt,	///< prognostic variables
			sweet::SphereData_Spectral &io_div,	///< prognostic variables

			double i_dt = 0,
			double i_simulation_timestamp = -1
	);


	virtual ~PDESWESphereTS_l_exp_n_etdrk();
};

#endif
