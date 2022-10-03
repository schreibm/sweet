/*
 * Author: Martin Schreiber <SchreiberX@gmail.com>
 */

//////////#if !SWEET_PARAREAL
//////////#	error "Parareal not activated"
//////////#endif

#if ! SWEET_SCALAR_COMPLEX
	#define typename_scalar double
#else
	#define typename_scalar std::complex<double>
#endif

#include <limits>
#include <stdlib.h>

#if SWEET_PARAREAL || SWEET_XBRAID
#include <parareal/Parareal.hpp>
#include <parareal/Parareal_GenericData.hpp>
#include <parareal/Parareal_GenericData_Scalar.hpp>
#endif

#if SWEET_PARAREAL
#include <parareal/Parareal_Controller.hpp>
#endif

#if SWEET_XBRAID
	#include <xbraid/XBraid_sweet_lib.hpp>
#endif

#include <cmath>
#include <sweet/SimulationVariables.hpp>

#include "ode_scalar_timeintegrators/ODE_Scalar_TimeSteppers.hpp"

SimulationVariables simVars;

double param_parareal_fine_dt = -1;
double param_function_y0_real = 0.123;
double param_function_y0_imag = 0.;
double param_function_L = 0.1;
double param_function_N = 1.0;
std::string ode_model = "ode1";

double param_fine_timestepping_solution = std::numeric_limits<double>::infinity();

/*
 * ODE implementation of the Parareal algorithm to test implementations.
 *
 * Usage of the program:
 * --parareal-fine-dt=0.0001 --parareal-enabled=1 --parareal-coarse-slices=10 -t 10 --parareal-convergence-threshold=0.0001 --parareal-function-param-a=0.3 --parareal-function-param-b=1.0 --parareal-function-param-y0=0.123
 *
 */

//////////class ODE_Scalar_TS_interface
//////////{
//////////private:
//////////	double u_prev;
//////////
//////////public:
//////////	void run_timestep(
//////////			double &io_y,			///< prognostic variables
//////////
//////////			double i_dt,		///< time step size
//////////			double i_sim_timestamp
//////////	)
//////////	{
//////////		double a = param_parareal_function_L;
//////////		double b = param_parareal_function_N;
//////////
//////////		io_y += i_dt * (a * std::sin(io_y) + b * std::sin(i_sim_timestamp));
//////////	}
//////////
//////////#if (SWEET_PARAREAL && SWEET_PARAREAL_SCALAR) || (SWEET_XBRAID && SWEET_XBRAID_SCALAR)
//////////	void run_timestep(
//////////			Parareal_GenericData* io_data,
//////////
//////////			double i_dt,		///< time step size
//////////			double i_sim_timestamp
//////////	)
//////////	{
//////////		double y = io_data->get_pointer_to_data_Scalar()->simfields[0];
//////////
//////////		run_timestep(y,
//////////				i_dt,
//////////				i_sim_timestamp
//////////			);
//////////
//////////		io_data->get_pointer_to_data_Scalar()->simfields[0] = y;
//////////
//////////	}
//////////
//////////	// for parareal SL (not needed here)
//////////	void set_previous_solution(
//////////			Parareal_GenericData* i_data
//////////	)
//////////	{
//////////		u_prev = i_data->get_pointer_to_data_Scalar()->simfields[0];
//////////	};
//////////#endif
//////////};
//////////
//////////
//////////
//////////
//////////class ODE_Scalar_TimeSteppers
//////////{
//////////public:
//////////	ODE_Scalar_TS_interface *master = nullptr;
//////////
//////////	ODE_Scalar_TimeSteppers()
//////////	{
//////////	}
//////////
//////////	void reset()
//////////	{
//////////		if (master != nullptr)
//////////		{
//////////			delete master;
//////////			master = nullptr;
//////////		}
//////////	}
//////////
//////////	void setup(
//////////			const std::string &i_timestepping_method,
//////////			int &i_timestepping_order,
//////////
//////////			SimulationVariables &i_simVars
//////////	)
//////////	{
//////////		reset();
//////////		master = new ODE_Scalar_TS_interface;
//////////	}
//////////
//////////	~ODE_Scalar_TimeSteppers()
//////////	{
//////////		reset();
//////////	}
//////////};



class SimulationInstance
{


private:
	typename_scalar prog_u;
	SimulationVariables* simVars;

public:
	ODE_Scalar_TimeSteppers<typename_scalar>* timeSteppers = nullptr;

public:
	SimulationInstance(SimulationVariables* i_simVars)
		: simVars(i_simVars)
	{
		timeSteppers = new ODE_Scalar_TimeSteppers<typename_scalar>;
		timeSteppers->setup(
					simVars->disc.timestepping_method,
					simVars->disc.timestepping_order,
					*simVars
				);
	}

	~SimulationInstance()
	{
		if (timeSteppers)
		{
			delete timeSteppers;
			timeSteppers = nullptr;
		}
	}

public:
	void run()
	{
		// reset simulation time
		simVars->timecontrol.current_simulation_time = 0;
		simVars->timecontrol.current_timestep_nr = 0;
#if !SWEET_SCALAR_COMPLEX
		this->prog_u = param_function_y0_real;
#else
		this->prog_u = std::complex<double>(param_function_y0_real, param_function_y0_imag);
#endif

		this->do_output();
		while (true)
		{
			this->timeSteppers->master->run_timestep(this->prog_u,
					simVars->timecontrol.current_timestep_size,
					simVars->timecontrol.current_simulation_time
				);

			simVars->timecontrol.current_simulation_time += simVars->timecontrol.current_timestep_size;
			simVars->timecontrol.current_timestep_nr++;

			this->do_output();

			if (this->should_quit())
				break;
		}

	}


public:

///////////	/**
///////////	 * ODE to simulate
///////////	 */
///////////	double f_dt(double y, double t)
///////////	{
///////////		double a = param_parareal_function_L;
///////////		double b = param_parareal_function_N;
///////////
///////////#if 0
///////////		double y0 = param_parareal_function_y0;
///////////		return b*sin(t) / (1 - a*sin(y0));
///////////#else
///////////		return a * std::sin(y) + b * std::sin(t);
///////////#endif
///////////	}


	bool should_quit()
	{
		if (
				simVars->timecontrol.max_timesteps_nr != -1 &&
				simVars->timecontrol.max_timesteps_nr <= simVars->timecontrol.current_timestep_nr
		)
			return true;

		if (!std::isinf(simVars->timecontrol.max_simulation_time))
			if (simVars->timecontrol.max_simulation_time <= simVars->timecontrol.current_simulation_time+simVars->timecontrol.max_simulation_time*1e-10)	// care about roundoff errors with 1e-10
				return true;

		return false;
	}


	void do_output()
	{
		char buffer[1024];

		const char* filename_template = "output_%s_t%020.8f.csv";
		sprintf(buffer, filename_template, "prog_u", simVars->timecontrol.current_simulation_time);

		std::ofstream file(buffer, std::ios_base::trunc);
		file << std::setprecision(16);

		file << "#SWEET" << std::endl;
		file << "#FORMAT ASCII" << std::endl;
		file << "#PRIMITIVE SCALAR" << std::endl;

		file << this->prog_u;

		file.close();
	}


};



int main(int i_argc, char *i_argv[])
{

#if SWEET_MPI

	MPI_Init(&i_argc, &i_argv);

	int mpi_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

#endif

	const char *bogus_var_names[] = {
		"parareal-fine-dt",
		"function-param-y0-real",
		"function-param-y0-imag",
		"function-param-L",
		"function-param-N",
		"ode-model",
		nullptr
	};

	param_parareal_fine_dt = -1;
	param_function_y0_real = 0.123;
	param_function_y0_imag = 0.;
	param_function_L = 1.0;
	param_function_N = 0.1;
	ode_model = "ode1";

	if (!simVars.setupFromMainParameters(i_argc, i_argv, bogus_var_names))
	{
		///std::cout << "	--parareal-fine-dt				Fine time stepping size" << std::endl;
		std::cout << "	--function-param-y0-real	Parameter 'Re(y0)' (initial condition) for function y(t=0)" << std::endl;
		std::cout << "	--function-param-y0-imag	Parameter 'Im(y0)' (initial condition) for function y(t=0)" << std::endl;
		std::cout << "	--function-param-L		Parameter 'a_L' for function 'a_L*f_L(y,dt,t) + a_N*f_N(y,dt,t)" << std::endl;
		std::cout << "	--function-param-N		Parameter 'a_N' for function 'a_L*f_L(y,dt,t) + a_N*f_N(y,dt,t)" << std::endl;
		std::cout << "	--ode-model			string naming the ODE model" << std::endl;
		std::cout << "Unknown option detected" << std::endl;
		exit(-1);
	}


	param_parareal_fine_dt = simVars.timecontrol.current_timestep_size;
	//if (simVars.bogus.var[0] != "")
	//	param_parareal_fine_dt = atof(simVars.bogus.var[0].c_str());
	if (simVars.bogus.var[1] != "")
		param_function_y0_real = atof(simVars.bogus.var[1].c_str());
	else
		simVars.bogus.var[1] = std::to_string(param_function_y0_real);

	if (simVars.bogus.var[2] != "")
		param_function_y0_imag = atof(simVars.bogus.var[2].c_str());
	else
		simVars.bogus.var[2] = std::to_string(param_function_y0_imag);

	if (simVars.bogus.var[3] != "")
		param_function_L = atof(simVars.bogus.var[3].c_str());
	else
		simVars.bogus.var[3] = std::to_string(param_function_L);

	if (simVars.bogus.var[4] != "")
		param_function_N = atof(simVars.bogus.var[4].c_str());
	else
		simVars.bogus.var[4] = std::to_string(param_function_N);

	if (simVars.bogus.var[5] != "")
		ode_model = simVars.bogus.var[5];
	else
		simVars.bogus.var[5] = ode_model;


	if (param_parareal_fine_dt <= 0)
	{
		std::cout << "Specify fine time step size via --parareal-fine-dt=[value]" << std::endl;
		return -1;
	}

#if (!SWEET_PARAREAL && !SWEET_XBRAID)

	SimulationInstance* sim = new SimulationInstance(&simVars);

	sim->run();

	delete sim;

#endif

#if SWEET_PARAREAL
	typename_scalar aux;
	if (!simVars.parareal.enabled)
	{
		std::cout << "Activate parareal mode via --parareal0enable=1" << std::endl;
		return -1;
	}

	std::cout << "Running parareal" << std::endl;
	std::cout << " + fine dt: " << param_parareal_fine_dt << std::endl;
	std::cout << " + initial condition: y0 = (" << param_function_y0_real << ", " << param_function_y0_imag << ")" << std::endl;
	std::cout << " + function df(y,t)/dt = " << param_function_L << "*sin(y) + " << param_function_N << "*sin(t)" << std::endl;

	if (simVars.parareal.enabled)
	{

		ODE_Scalar_TimeSteppers<typename_scalar>* timeSteppersFine = new ODE_Scalar_TimeSteppers<typename_scalar>;
		ODE_Scalar_TimeSteppers<typename_scalar>* timeSteppersCoarse = new ODE_Scalar_TimeSteppers<typename_scalar>;

		/*
		 * Allocate parareal controller and provide class
		 * which implement the parareal features
		 */
		Parareal_Controller<ODE_Scalar_TimeSteppers<typename_scalar>, 1> parareal_Controller(&simVars,
											timeSteppersFine,
											timeSteppersCoarse);

		std::cout << simVars.parareal.path_ref_csv_files << std::endl;
		std::cout << &simVars.parareal.path_ref_csv_files << std::endl;
		std::cout << &simVars.parareal << std::endl;
		std::cout << &simVars << std::endl;
		// setup controller. This initializes several simulation instances
		parareal_Controller.setup();

		// execute the simulation
		parareal_Controller.run();

		delete timeSteppersFine;
		delete timeSteppersCoarse;
	}
#elif SWEET_XBRAID

	if (simVars.xbraid.xbraid_enabled)
	{

		MPI_Comm comm = MPI_COMM_WORLD;
		MPI_Comm comm_x, comm_t;

		int nt = (int) (simVars.timecontrol.max_simulation_time / simVars.timecontrol.current_timestep_size);
                if (nt * simVars.timecontrol.current_timestep_size < simVars.timecontrol.max_simulation_time - 1e-10)
			nt++;
		sweet_BraidApp app(MPI_COMM_WORLD, mpi_rank, 0., simVars.timecontrol.max_simulation_time, nt, &simVars);

		if( simVars.xbraid.xbraid_run_wrapper_tests)
		{

			app.setup();

			BraidUtil braid_util;
			int test = braid_util.TestAll(&app, comm, stdout, 0., simVars.timecontrol.current_timestep_size, simVars.timecontrol.current_timestep_size * 2);
			if (test == 0)
				SWEETError("Tests failed!");
			else
				std::cout << "Tests successful!" << std::endl;

		}
		else
		{
			BraidCore core(MPI_COMM_WORLD, &app);
			app.setup(core);
			// Run Simulation
			core.Drive();
		}

	}



#endif

#if SWEET_MPI
	MPI_Finalize();
#endif


	return 0;
}
