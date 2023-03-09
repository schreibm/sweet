/*
 * Author: Martin SCHREIBER <schreiberx@gmail.com>
 */

#ifndef SRC_PROGRAMS_SWE_SPHERE_TS_LG_IRK_HPP_
#define SRC_PROGRAMS_SWE_SPHERE_TS_LG_IRK_HPP_



#include <complex>
#include <sweet/core/sphere/SphereData_Spectral.hpp>
#include <sweet/core/sphere/SphereOperators.hpp>
#include <sweet/core/shacks/ShackDictionary.hpp>

#include "helpers/SWESphBandedMatrixPhysicalReal.hpp"
#include "PDESWESphereTS_BaseInterface.hpp"
#include "PDESWESphereTS_lg_erk.hpp"


class PDESWESphereTS_lg_irk	: public PDESWESphereTS_BaseInterface
{
public:
	bool setup_auto(sweet::SphereOperators *io_ops);

public:
	bool setup(
		sweet::SphereOperators *io_ops,
		int i_timestep_order,
		double i_timestep_size
	);

public:
	bool setup_main(
		sweet::SphereOperators *io_ops,
		int i_timestep_order,
		double i_timestep_size,
		double i_crank_nicolson_damping_factor
	);

public:
	bool implementsTimesteppingMethod(const std::string &i_timestepping_method);
	std::string getIDString();


private:
	/// SPH configuration
	//const sweet::SphereData_Config *sphereDataConfig;

	PDESWESphereTS_lg_erk *lg_erk = nullptr;

	/// alpha/beta (time step related component for implicit solver)
	double alpha;
	double beta;

	/// Crank-Nicolson damping factor
	double crank_nicolson_damping_factor = 0.5;

	/// timestep size
	double timestep_size;

	/// earth radius
	double r;

	/// inverse of earth radius
	double inv_r;

	/// Average geopotential
	double gh;

public:
	PDESWESphereTS_lg_irk();


public:
	void update_coefficients();



public:
	void runTimestep(
		sweet::SphereData_Spectral &io_phi,		///< prognostic variables
		sweet::SphereData_Spectral &io_vort,	///< prognostic variables
		sweet::SphereData_Spectral &io_div,		///< prognostic variables

		double i_fixed_dt = 0,
		double i_simulation_timestamp = -1
	);

	virtual ~PDESWESphereTS_lg_irk();
};


#endif
