/*
 * ODE_Scalar_TS_ln_exp.hpp
 *
 *  Created on: 30 Sep 2022
 *      Author: Joao Steinstraesser <joao.steinstraesser@usp.br>
 */

#ifndef SRC_PROGRAMS_ODE_SCALAR_TS_L_EXP_HPP_
#define SRC_PROGRAMS_ODE_SCALAR_TS_L_EXP_HPP_

#include <limits>
#include <string>
#include <complex>
#include <sweet/SimulationVariables.hpp>

#include "../ode_scalar_timeintegrators/ODE_Scalar_TS_interface.hpp"
#include "../ode_scalar_timeintegrators/ODE_Scalar_TS_l_direct.hpp"


#ifndef SWEET_BENCHMARK_TIMINGS
	#define SWEET_BENCHMARK_TIMINGS	0
#endif

#if SWEET_BENCHMARK_TIMINGS
#	include <sweet/Stopwatch.hpp>
#endif


#if SWEET_MPI
#	include <mpi.h>
#endif

template <typename T>
class ODE_Scalar_TS_l_exp	: public ODE_Scalar_TS_interface<T>
{
	SimulationVariables &simVars;
	EXP_SimulationVariables *expSimVars;

	ODE_Scalar_TS_interface<T> *master = nullptr;
	ODE_Scalar_TS_l_direct<T> ts_l_direct;

public:
	ODE_Scalar_TS_l_exp(
			SimulationVariables &i_simVars
		)
		:
		simVars(i_simVars),
		ts_l_direct(i_simVars)
	{
	}


	void setup(
			EXP_SimulationVariables &i_exp,
			const std::string &i_function_name,
			double i_timestep_size
	)
	{
		ts_l_direct.setup(i_function_name);
	}

	void setup(
			std::vector<double> i_L,
			std::vector<double> i_N,
			std::vector<double> i_extra,
			std::string i_model
		)
	{
		this->param_function_L = i_L;
		this->param_function_N = i_N;
		this->param_function_N = i_extra;
		this->model = i_model;

		ts_l_direct.setup(this->param_function_L, this->param_function_N, this->param_function_extra, this->model);

		master = &(ODE_Scalar_TS_interface<T>&)*(&ts_l_direct);
		master->setup(
				this->simVars.bogus.var[3],
				this->simVars.bogus.var[4],
				this->simVars.bogus.var[5],
				this->simVars.bogus.var[6]
			);
	}



	void run_timestep(
			///T &io_u,	///< prognostic variables
			ScalarDataArray &io_u,	///< prognostic variables

			double i_dt = 0,
			double i_simulation_timestamp = -1
	)
	{
		ts_l_direct.run_timestep(io_u, i_dt, i_simulation_timestamp);
	}



	void run_timestep(
			const ScalarDataArray &i_u,	///< prognostic variables

			ScalarDataArray &o_u,

			double i_dt = 0,
			double i_simulation_timestamp = -1
	)
	{
		o_u = i_u;
		ts_l_direct.run_timestep(o_u, i_dt, i_simulation_timestamp);
	}



public:

	virtual ~ODE_Scalar_TS_l_exp()
	{
	}
};

#endif /* SRC_PROGRAMS_ODE_SCALAR_EXP_ODE_SCALAR_TS_LN_ERK_HPP_ */
