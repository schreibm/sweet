/*
 * Xbraid_sweet_lib.hpp
 *
 *  Created on: 10 Jun 2022
 *      Author: Joao Steinstraesser <joao.steinstraesser@usp.br>
 *
 */

#ifndef SRC_INCLUDE_XBRAID_SWEET_LIB_HPP_
#define SRC_INCLUDE_XBRAID_SWEET_LIB_HPP_

#include <braid.hpp>
#include <common_pint/PInT_Common.hpp>
#include <parareal/Parareal_GenericData.hpp>

#if SWEET_XBRAID_SCALAR
	#include <parareal/Parareal_GenericData_Scalar.hpp>
	#include "src/programs/ode_scalar_timeintegrators/ODE_Scalar_TimeSteppers.hpp"
	typedef ODE_Scalar_TimeSteppers t_tsmType;
	#define N 1
#elif SWEET_XBRAID_PLANE
	#include <parareal/Parareal_GenericData_PlaneData_Spectral.hpp>
	#if SWEET_XBRAID_PLANE_BURGERS
		typedef Burgers_Plane_TimeSteppers t_tsmType;
		#define N 2
	#elif SWEET_XBRAID_PLANE_SWE
		typedef SWE_Plane_TimeSteppers t_tsmType;
		#define N 3
	#endif
#elif SWEET_XBRAID_SPHERE
	#include <parareal/Parareal_GenericData_SphereData_Spectral.hpp>
	typedef SWE_Sphere_TimeSteppers t_tsmType;
	#define N 3
#endif

#if SWEET_XBRAID_PLANE
	// Grid Mapping (staggered grid)
	PlaneDataGridMapping gridMapping;
#endif



class sweet_BraidVector;
class sweet_BraidApp;



/* --------------------------------------------------------------------
 * XBraid vector 
 * Stores the state of the simulation for a given time step
 * Define BraidVector, can contain anything, and be named anything
 * --> Put all time-dependent information here
 * -------------------------------------------------------------------- */
class sweet_BraidVector
{
public:
	Parareal_GenericData*	data = nullptr;
	int level;

#if SWEET_XBRAID_PLANE
	PlaneDataConfig* planeDataConfig;
#elif SWEET_XBRAID_SPHERE
	SphereData_Config* sphereDataConfig;
#endif


	sweet_BraidVector(
#if SWEET_XBRAID_PLANE
				PlaneDataConfig* i_planeDataConfig,
#elif SWEET_XBRAID_SPHERE
				SphereData_Config* i_sphereDataConfig,
#endif
				int i_level
	)
#if SWEET_XBRAID_PLANE
		: planeDataConfig(i_planeDataConfig),
#elif SWEET_XBRAID_SPHERE
		: sphereDataConfig(i_sphereDataConfig),
#endif
		level(i_level)
	{
		this->allocate_data();
	}

	virtual ~sweet_BraidVector()
	{
		if (data)
		{
			delete data;
			data = nullptr;
		}
	}

	sweet_BraidVector(const sweet_BraidVector &i_vector)
	{
#if SWEET_XBRAID_PLANE
		this->planeDataConfig = i_vector.planeDataConfig;
#elif SWEET_XBRAID_SPHERE
		this->sphereDataConfig = i_vector.sphereDataConfig;
#endif
		*this->data = *i_vector.data;
		this->level = i_vector.level;
	};

	sweet_BraidVector& operator=(const sweet_BraidVector &i_vector)
	{
#if SWEET_XBRAID_PLANE
		this->planeDataConfig = i_vector.planeDataConfig;
#elif SWEET_XBRAID_SPHERE
		this->sphereDataConfig = i_vector.sphereDataConfig;
#endif
		*this->data = *i_vector.data;
		this->level = i_vector.level;
		return *this;
	};

	sweet_BraidVector operator+(
			const sweet_BraidVector &i_vector
	)	const
	{
#if SWEET_XBRAID_SCALAR
		sweet_BraidVector out;
#elif SWEET_XBRAID_PLANE
		sweet_BraidVector out(this->planeDataConfig, this->level);
#elif SWEET_XBRAID_SPHERE
		sweet_BraidVector out(this->sphereDataConfig, this->level);
#endif
		*out.data = *this->data;
		*out.data += *i_vector.data;
		return out;
	}

	sweet_BraidVector operator*(
			const double i_value
	)	const
	{
#if SWEET_XBRAID_SCALAR
		sweet_BraidVector out;
#elif SWEET_XBRAID_PLANE
		sweet_BraidVector out(this->planeDataConfig, this->level);
#elif SWEET_XBRAID_SPHERE
		sweet_BraidVector out(this->sphereDataConfig, this->level);
#endif
		*out.data = *this->data;
		*out.data *= i_value;
		return out;
	}


	void allocate_data()
	{
#if SWEET_XBRAID_SCALAR
		{
			data = new Parareal_GenericData_Scalar<N>;
			data->allocate_data();
			//this->set_time(this->timeframe_end);
		}

#elif SWEET_XBRAID_PLANE
		{
			data = new Parareal_GenericData_PlaneData_Spectral<N>;
			data->setup_data_config(this->planeDataConfig);
			data->allocate_data();
			//this->setup_data_config(this->planeDataConfig);
			//this->set_time(this->timeframe_end);
		}

#elif SWEET_XBRAID_SPHERE
		{
			data = new Parareal_GenericData_SphereData_Spectral<N>;
			data->setup_data_config(this->sphereDataConfig);
			data->allocate_data();
			//this->setup_data_config(this->sphereDataConfig);
			//this->set_time(this->timeframe_end);
		}
#endif
	}



};


/**
 * operator to support operations such as:
 *
 * 1.5 * arrayData;
 *
 * Otherwise, we'd have to write it as arrayData*1.5
 *
 */
inline
static
sweet_BraidVector operator*(
		const double i_value,
		const sweet_BraidVector &i_vector
)
{
	return i_vector * i_value;
}



// Wrapper for BRAID's App object·
// --> Put all time INDEPENDENT information here
class sweet_BraidApp
			: public BraidApp, PInT_Common
{

public:

	///SimulationVariables*		simVars;
	double				dt;
	std::vector<t_tsmType*>		timeSteppers;

	int			size_buffer;

	int rank;

	// Reference solutions (online error computation)
	std::vector<sweet_BraidVector*> xbraid_data_ref_exact;
	std::vector<sweet_BraidVector*> xbraid_data_fine_exact;

	// Solution from previous timestep (for SL)
	std::vector<std::vector<sweet_BraidVector*>> sol_prev; // sol_prev[level][timestep]
	std::vector<std::vector<int>> sol_prev_iter; // store iteration in which the solution has been stored

	// Timestepping method and orders for each level
	std::vector<std::string> tsms;
	std::vector<int> tsos;
	std::vector<int> tsos2;

	// was the output of the time step already done for this simulation state?
	////double timestep_last_output_simtime;

public:

	// Constructor·
	sweet_BraidApp( MPI_Comm		i_comm_t,
			int			i_rank,
			double			i_tstart,
			double			i_tstop,
			int 			i_ntime,
			SimulationVariables*	i_simVars
#if SWEET_XBRAID_PLANE
			,
			//PlaneDataConfig* i_planeDataConfig,
			//PlaneOperators* i_op_plane
			std::vector<PlaneDataConfig*> i_planeDataConfig,
			std::vector<PlaneOperators*> i_op_plane
#elif SWEET_XBRAID_SPHERE
			,
			///SphereData_Config* i_sphereDataConfig,
			///SphereOperators_SphereData* i_op_sphere
			std::vector<SphereData_Config*> i_sphereDataConfig,
			std::vector<SphereOperators_SphereData*> i_op_sphere
#endif
			)
		:
			BraidApp(i_comm_t, i_tstart, i_tstop, i_ntime),
			rank(i_rank)
	{
		this->simVars = i_simVars;

#if SWEET_XBRAID_PLANE
			this->planeDataConfig = i_planeDataConfig;
			this->op_plane = i_op_plane;
#elif SWEET_XBRAID_SPHERE
			this->sphereDataConfig = i_sphereDataConfig;
			this->op_sphere = i_op_sphere;
#endif

		///this->xbraid_data_ref_exact = this->create_new_vector();
		///this->xbraid_data_fine_exact = this->create_new_vector();

		// start at one second in the past to ensure output at t=0
		////this->timestep_last_output_simtime = i_tstart-1.0;
		////this->simVars->iodata.output_next_sim_seconds = 0;
	}

	virtual ~sweet_BraidApp()
	{

		for (std::vector<t_tsmType*>::iterator it = this->timeSteppers.begin();
							it != this->timeSteppers.end();
							it++)
			if (*it)
			{
				delete *it;
				*it = nullptr;
			}

		for (std::vector<sweet_BraidVector*>::iterator it = this->xbraid_data_ref_exact.begin();
								it != this->xbraid_data_ref_exact.end();
								it++)
			if (*it)
			{
				delete *it;
				*it = nullptr;
			}

		for (std::vector<sweet_BraidVector*>::iterator it = this->xbraid_data_fine_exact.begin();
								it != this->xbraid_data_fine_exact.end();
								it++)
			if (*it)
			{
				delete *it;
				*it = nullptr;
			}

		for (std::vector<std::vector<sweet_BraidVector*>>::iterator it = this->sol_prev.begin();
										it != this->sol_prev.end();
										it++)
			for (std::vector<sweet_BraidVector*>::iterator it2 = it->begin();
										it2 != it->end();
										it2++)
				if (*it2)
				{
					delete *it2;
					*it2 = nullptr;
				}

	}

public:
	void setup(BraidCore& i_core)
	{


		/////////////////////////////////////////////////
		// get parameters from simVars and set to Core //
		/////////////////////////////////////////////////

		i_core.SetMaxLevels(this->simVars->xbraid.xbraid_max_levels);
		/////i_core.SetIncrMaxLevels();

		i_core.SetSkip(this->simVars->xbraid.xbraid_skip);

		i_core.SetMinCoarse(this->simVars->xbraid.xbraid_min_coarse);

		///i_core.SetRelaxOnlyCG(this->simVars->xbraid.xbraid_relax_only_cg);

		i_core.SetNRelax(-1, this->simVars->xbraid.xbraid_nrelax);
		if (this->simVars->xbraid.xbraid_nrelax0 > -1)
			i_core.SetNRelax(0, this->simVars->xbraid.xbraid_nrelax0);

		i_core.SetAbsTol(this->simVars->xbraid.xbraid_tol);
		i_core.SetRelTol(this->simVars->xbraid.xbraid_tol);

		i_core.SetTemporalNorm(this->simVars->xbraid.xbraid_tnorm);

		i_core.SetCFactor(-1, this->simVars->xbraid.xbraid_cfactor);
		if (this->simVars->xbraid.xbraid_cfactor0 > -1)
			i_core.SetCFactor(0, this->simVars->xbraid.xbraid_cfactor0);

		///i_core.SetiPeriodic(this->simVars->xbraid.xbraid_periodic);

		////i_core.SetResidual();

		i_core.SetMaxIter(this->simVars->xbraid.xbraid_max_iter);

		i_core.SetPrintLevel(this->simVars->xbraid.xbraid_print_level);

		i_core.SetSeqSoln(this->simVars->xbraid.xbraid_use_seq_soln);

		i_core.SetAccessLevel(this->simVars->xbraid.xbraid_access_level);

		i_core.SetNFMG(this->simVars->xbraid.xbraid_fmg);

		//i_core.SetiNFMGVcyc(this->simVars->xbraid.xbraid_fmgvcyc);

		i_core.SetStorage(this->simVars->xbraid.xbraid_storage);

		//i_core.SetRevertedRanks(this->simVars->xbraid.xbraid_reverted_ranks);

		////i_core.SetRefine(this->simVars->xbraid.xbraid_refine);
		///i_core.SetMaxRefinements(this->simVars->xbraid.xbraid_max_Refinements);


		this->setup();
	}

public:
	void setup()
	{

		PInT_Common::setup();

		////////////////////////////////////
		// get tsm and tso for each level //
		////////////////////////////////////
		this->tsms = this->getTimeSteppingMethodFromParameters();
		this->tsos = this->getTimeSteppingOrderFromParameters(1);
		this->tsos2 = this->getTimeSteppingOrderFromParameters(2);

		// create a timeSteppers instance for each level
		for (int level = 0; level < this->simVars->xbraid.xbraid_max_levels; level++)
		{

			// Configure timesteppers with the correct timestep for this level
			double dt = this->simVars->timecontrol.current_timestep_size;
			this->simVars->timecontrol.current_timestep_size *= std::pow(this->simVars->xbraid.xbraid_cfactor, level);

			std::cout << "Timestep size at level " << level << " : " << this->simVars->timecontrol.current_timestep_size << std::endl;

#if SWEET_XBRAID_SCALAR
			ODE_Scalar_TimeSteppers* tsm = new ODE_Scalar_TimeSteppers;
			tsm->setup(
					//tsms[level],
					//tsos[level],
					*this->simVars
				);
#elif SWEET_XBRAID_PLANE
	#if SWEET_XBRAID_PLANE_SWE
			SWE_Plane_TimeSteppers* tsm = new SWE_Plane_TimeSteppers;
			tsm->setup(
					tsms[level],
					tsos[level],
					tsos2[level],
					*this->op_plane[level],
					*this->simVars
				);
	#elif SWEET_XBRAID_PLANE_BURGERS
			Burgers_Plane_TimeSteppers* tsm = new Burgers_Plane_TimeSteppers;
			tsm->setup(
					tsms[level],
					tsos[level],
					tsos2[level],
					*this->op_plane[level],
					*this->simVars
				);
	#endif
#elif SWEET_XBRAID_SPHERE

			SWE_Sphere_TimeSteppers* tsm = new SWE_Sphere_TimeSteppers;
			tsm->setup(
						tsms[level],
						*this->op_sphere[level],
						*this->simVars
					);

#endif

			// get back the original timestep size
			this->simVars->timecontrol.current_timestep_size = dt;

			this->timeSteppers.push_back(tsm);

		}


		// get buffer size
#if SWEET_XBRAID_SCALAR
		this->size_buffer = N * sizeof(double);
#elif SWEET_XBRAID_PLANE
		this->size_buffer = N * planeDataConfig[0]->spectral_array_data_number_of_elements * sizeof(std::complex<double>);
#elif SWEET_XBRAID_SPHERE
		this->size_buffer = N * sphereDataConfig[0]->spectral_array_data_number_of_elements * sizeof(std::complex<double>);
#endif

		// create vectors for storing solutions from previous timestep (SL)
		for (int i = 0; i < this->simVars->xbraid.xbraid_max_levels; i++)
		{
			std::vector<sweet_BraidVector*> v = {};
			this->sol_prev.push_back(v);

			std::vector<int> w = {};
			this->sol_prev_iter.push_back(w);
		}

	}


private:
	/*
	 * Get timestepping methods for each of the N levels
	 * Input string must contain 1, 2 or N tsm names separated by comma:
	 *  - 1 tsm name: same tsm for all levels;
	 *  - 2 tsm names: first one is for level 0 (finest); all coarse levels use the second one
	 *  - N tsm names: i-th level uses the i-th tsm.
	 */
	std::vector<std::string> getTimeSteppingMethodFromParameters()
	{
		std::vector<std::string> tsm = {};
		std::stringstream all_tsm = std::stringstream(this->simVars->xbraid.xbraid_timestepping_method);

		while (all_tsm.good())
		{
			std::string str;
			getline(all_tsm, str, ',');
			tsm.push_back(str);
		}

		if ( ! (tsm.size() == 1 || tsm.size() == 2 || tsm.size() == this->simVars->xbraid.xbraid_max_levels ) )
			SWEETError("xbraid_timestepping_method must contain 1, 2 or N timestepping names.");

		// all levels use same tsm
		if (tsm.size() == 1)
			for (int level = 1; level < this->simVars->xbraid.xbraid_max_levels; level++)
				tsm.push_back(tsm[0]);

		// all coarse levels use same tsm
		if (tsm.size() == 2)
			for (int level = 2; level < this->simVars->xbraid.xbraid_max_levels; level++)
				tsm.push_back(tsm[1]);


		return tsm;
	}

	/*
	 * Get timestepping order for each of the N levels
	 * Input string must contain 1, 2 or N orders separated by comma:
	 *  - 1 tso: same tso for all levels;
	 *  - 2 tso: first one is for level 0 (finest); all coarse levels use the second one
	 *  - N tso: i-th level uses the i-th tso.
	 */
	std::vector<int> getTimeSteppingOrderFromParameters(int o)
	{
		std::vector<int> tso = {};
		std::stringstream all_tso;
		if (o == 1)
			all_tso = std::stringstream(this->simVars->xbraid.xbraid_timestepping_order);
		else if (o == 2)
			all_tso = std::stringstream(this->simVars->xbraid.xbraid_timestepping_order2);
		else
			SWEETError("Wrong parameter for getting timestepping order.");

		while (all_tso.good())
		{
			std::string str;
			getline(all_tso, str, ',');
			tso.push_back(stoi(str));
		}

		if ( ! (tso.size() == 1 || tso.size() == 2 || tso.size() == this->simVars->xbraid.xbraid_max_levels ) )
			SWEETError("xbraid_timestepping_order must contain 1, 2 or N timestepping orders.");

		// all levels use same tso
		if (tso.size() == 1)
			for (int level = 1; level < this->simVars->xbraid.xbraid_max_levels; level++)
				tso.push_back(tso[0]);

		// all coarse levels use same tso
		if (tso.size() == 2)
			for (int level = 2; level < this->simVars->xbraid.xbraid_max_levels; level++)
				tso.push_back(tso[1]);

		return tso;
	}



public:
	sweet_BraidVector* create_new_vector(int i_level)
	{
#if SWEET_XBRAID_SCALAR
		sweet_BraidVector* U = new sweet_BraidVector(i_level);
#elif SWEET_XBRAID_PLANE
		sweet_BraidVector* U = new sweet_BraidVector(this->planeDataConfig[i_level], i_level);
#elif SWEET_XBRAID_SPHERE
		sweet_BraidVector* U = new sweet_BraidVector(this->sphereDataConfig[i_level], i_level);
#endif
		return U;
	}

private:
	void store_prev_solution(
					sweet_BraidVector* i_U,
					int i_time_id,
					int i_level,
					int iter
				)
	{
		// if not SL scheme: nothing to do
		if ( std::find(this->SL_tsm.begin(), this->SL_tsm.end(), this->tsms[i_level]) == this->SL_tsm.end())
			return;

		// if solution has already been stored in this iteration: nothing to do
		if ( this->sol_prev_iter[i_level][i_time_id] == iter )
			return;
		///assert(this->sol_prev_iter[i_level][i_time_id] == iter - 1);

		// create vector if necessary
		if ( ! this->sol_prev[i_level][i_time_id] )
			this->sol_prev[i_level][i_time_id] = this->create_new_vector(i_level);

		// set solution
		*this->sol_prev[i_level][i_time_id] = *i_U;
		this->sol_prev_iter[i_level][i_time_id] = iter;
	}

	void set_prev_solution(
					sweet_BraidVector* i_U,
					int i_time_id,
					int i_level
				)
	{

		// if not SL scheme: nothing to do
		if (  std::find(this->SL_tsm.begin(), this->SL_tsm.end(), this->tsms[i_level]) == this->SL_tsm.end())
			return;

		// if t == 0 or prev solution not available
		// then: prev_solution = solution
		bool prev_sol_exists = true;

		if ( i_time_id == 0 )
			prev_sol_exists = false;
		if ( prev_sol_exists && (!this->sol_prev[i_level][i_time_id - 1]) )
			prev_sol_exists = false;

		if (prev_sol_exists)
			this->timeSteppers[i_level]->master->set_previous_solution(this->sol_prev[i_level][i_time_id - 1]->data);
		else
			this->timeSteppers[i_level]->master->set_previous_solution(i_U->data);
	}

public:
	/* --------------------------------------------------------------------
	 * Time integrator routine that performs the update
	 *   u_i = Phi_i(u_{i-1}) + g_i 
	 * 
	 * When Phi is called, u is u_{i-1}.
	 * The return value is that u is set to u_i upon completion
	 *
	 * Always receives and returns a solution defined on the finest spatial grid
	 * Spatial interpolation is performed if necessary
	 * -------------------------------------------------------------------- */
	braid_Int
	Step(
			braid_Vector		io_U,
			braid_Vector		i_ustop,
			braid_Vector		i_fstop,
			BraidStepStatus&	io_status
			)
	{

		// Vector defined in the finest level
		sweet_BraidVector* U = (sweet_BraidVector*) io_U;

		double tstart;             /* current time */
		double tstop;              /* evolve u to this time*/
		int level;
		int nlevels;
		int time_id;
		int iter;

		/* Grab status of current time step */
		io_status.GetTstartTstop(&tstart, &tstop);
		io_status.GetLevel(&level);
		io_status.GetNLevels(&nlevels);
		io_status.GetTIndex(&time_id);
		io_status.GetIter(&iter);

		// Vector defined in the current level (defined via interpolaiton if necessary)
		sweet_BraidVector* U_level = this->create_new_vector(level);

		// Interpolate to coarser grid in space if necessary
		if (this->simVars->xbraid.xbraid_spatial_coarsening && level > 0)
			U_level->data->restrict(*U->data);
		else if (level == 0)
			*U_level->data = *U->data;

		// create containers for prev solution
		if (this->sol_prev[level].size() == 0)
		{
			// store nt solutions (overestimated for coarse levels!)
			int nt;
			io_status.GetNTPoints(&nt);
			for (int i = 0; i < nt + 1; i++)
			{
				this->sol_prev[level].push_back(nullptr);
				this->sol_prev_iter[level].push_back(-1);
			}
				///this->sol_prev[level].push_back(this->create_new_vector());
		}

		// store solution for SL
		this->store_prev_solution(U_level, time_id, level, iter);

		// set prev solution for SL
		this->set_prev_solution(U_level, time_id, level);

		this->simVars->timecontrol.current_simulation_time = tstart;
		this->simVars->timecontrol.current_timestep_size = tstop - tstart;

		this->timeSteppers[level]->master->run_timestep(
								U_level->data,
								this->simVars->timecontrol.current_timestep_size,
								this->simVars->timecontrol.current_simulation_time
		);

		// Interpolate to finest grid in space if necessary
		if (this->simVars->xbraid.xbraid_spatial_coarsening && level > 0)
			U->data->pad_zeros(*U_level->data);
		else if (level == 0)
			*U->data = *U_level->data;

		/* Tell XBraid no refinement */
		io_status.SetRFactor(1);

		delete U_level;

		return 0;
	}

		/* --------------------------------------------------------------------
		 * -------------------------------------------------------------------- */

	virtual braid_Int
	Residual(
				braid_Vector			i_ustop,
				braid_Vector			o_r,
				BraidStepStatus&		io_status
		)
	{

		//braid_StepStatus& status = (braid_StepStatus&) io_status;

		double tstart;             /* current time */
		double tstop;              /* evolve u to this time*/
		int level;

		/* Grab status of current time step */
		io_status.GetTstartTstop(&tstart, &tstop);
	
		/* Grab level */
		io_status.GetLevel(&level);
	
		/* Set the new dt in the user's manager*/
		this->dt = tstop - tstart;


		sweet_BraidVector* u = (sweet_BraidVector*) i_ustop;
		sweet_BraidVector* r = (sweet_BraidVector*) o_r;

		return 0;
	}
	
	/* --------------------------------------------------------------------
	 * Create a vector object for a given time point.
	 * This function is only called on the finest level.
	 * -------------------------------------------------------------------- */
	braid_Int
	Init(
			double		i_t,
			braid_Vector*	o_U
			)
	{

		sweet_BraidVector* U = create_new_vector(0);

		if( i_t == this->tstart )
		{
			std::cout << "Setting initial solution " << std::endl;
	#if SWEET_XBRAID_SCALAR
			double u0 = atof(simVars->bogus.var[1].c_str());
			U->data->dataArrays_to_GenericData_Scalar(u0);
	
	#elif SWEET_XBRAID_PLANE
			PlaneData_Spectral t0_prog_h_pert(planeDataConfig[0]);
			PlaneData_Spectral t0_prog_u(planeDataConfig[0]);
			PlaneData_Spectral t0_prog_v(planeDataConfig[0]);
	
		#if SWEET_XBRAID_PLANE_SWE
			SWEPlaneBenchmarksCombined swePlaneBenchmarks;
			swePlaneBenchmarks.setupInitialConditions(t0_prog_h_pert, t0_prog_u, t0_prog_v, *simVars, *op_plane[0]);
		#elif SWEET_XBRAID_PLANE_BURGERS
			PlaneData_Physical t0_prog_u_phys(t0_prog_u.planeDataConfig[0]);
			PlaneData_Physical t0_prog_v_phys(t0_prog_v.planeDataConfig[0]);
			if (simVars->disc.space_grid_use_c_staggering)
			{
				t0_prog_u_phys.physical_update_lambda_array_indices(
							[&](int i, int j, double &io_data)
					{
						double x = (((double)i)/(double)simVars->disc.space_res_physical[0])*simVars->sim.plane_domain_size[0];
						double y = (((double)j+0.5)/(double)simVars->disc.space_res_physical[1])*simVars->sim.plane_domain_size[1];
						io_data = BurgersValidationBenchmarks::return_u(*simVars, x, y);
					}
				);
				t0_prog_v_phys.physical_update_lambda_array_indices(
							[&](int i, int j, double &io_data)
					{
					io_data = 0.0;
					}
				);
			}
			else
			{
				t0_prog_u_phys.physical_update_lambda_array_indices(
							[&](int i, int j, double &io_data)
					{
						double x = (((double)i+0.5)/(double)simVars->disc.space_res_physical[0])*simVars->sim.plane_domain_size[0];
						double y = (((double)j+0.5)/(double)simVars->disc.space_res_physical[1])*simVars->sim.plane_domain_size[1];
						io_data = BurgersValidationBenchmarks::return_u(*simVars, x, y);
					}
				);

				t0_prog_v_phys.physical_update_lambda_array_indices(
							[&](int i, int j, double &io_data)
					{
					io_data = 0.0;
					}
				);
			}
			t0_prog_u.loadPlaneDataPhysical(t0_prog_u_phys);
			t0_prog_v.loadPlaneDataPhysical(t0_prog_v_phys);
		#endif


			U->data->dataArrays_to_GenericData_PlaneData_Spectral(
		#if SWEET_XBRAID_PLANE_SWE
										t0_prog_h_pert,
		#endif
										t0_prog_u,
										t0_prog_v);

	#elif SWEET_XBRAID_SPHERE
			SphereData_Spectral t0_prog_phi_pert(sphereDataConfig[0]);
			SphereData_Spectral t0_prog_vrt(sphereDataConfig[0]);
			SphereData_Spectral t0_prog_div(sphereDataConfig[0]);

			BenchmarksSphereSWE sphereBenchmarks;
			sphereBenchmarks.setup(*simVars, *op_sphere[0]);
			sphereBenchmarks.master->get_initial_state(t0_prog_phi_pert, t0_prog_vrt, t0_prog_div);

			U->data->dataArrays_to_GenericData_SphereData_Spectral(t0_prog_phi_pert, t0_prog_vrt, t0_prog_div);
	#endif


		}
		else if (this->simVars->xbraid.xbraid_use_rand)
		{
			std::cout << "Setting random solution " << std::endl;
	#if SWEET_XBRAID_SCALAR
			double u0 = ((double)braid_Rand())/braid_RAND_MAX;
			U->data->dataArrays_to_GenericData_Scalar(u0);
	#elif SWEET_XBRAID_PLANE

		#if SWEET_XBRAID_PLANE_SWE
			PlaneData_Spectral t0_prog_h_pert(planeDataConfig[0]);
			PlaneData_Physical t0_prog_h_phys(planeDataConfig[0]);
		#endif

			PlaneData_Spectral t0_prog_u(planeDataConfig[0]);
			PlaneData_Spectral t0_prog_v(planeDataConfig[0]);

			PlaneData_Physical t0_prog_u_phys(planeDataConfig[0]);
			PlaneData_Physical t0_prog_v_phys(planeDataConfig[0]);

		#if SWEET_XBRAID_PLANE_SWE
			t0_prog_h_phys.physical_update_lambda_array_indices(
						[&](int i, int j, double &io_data)
				{
					io_data = this->simVars->sim.h0 + ((double)braid_Rand())/braid_RAND_MAX;
				}
			);
		#endif
			t0_prog_u_phys.physical_update_lambda_array_indices(
						[&](int i, int j, double &io_data)
				{
					io_data = ((double)braid_Rand())/braid_RAND_MAX;
				}
			);
			t0_prog_v_phys.physical_update_lambda_array_indices(
						[&](int i, int j, double &io_data)
				{
					io_data = ((double)braid_Rand())/braid_RAND_MAX;
				}
			);

		#if SWEET_XBRAID_PLANE_SWE
			t0_prog_h_pert.loadPlaneDataPhysical(t0_prog_h_phys);
		#endif
			t0_prog_u.loadPlaneDataPhysical(t0_prog_u_phys);
			t0_prog_v.loadPlaneDataPhysical(t0_prog_v_phys);

			U->data->dataArrays_to_GenericData_PlaneData_Spectral(
		#if SWEET_XBRAID_PLANE_SWE
										t0_prog_h_pert,
		#endif
										t0_prog_u,
										t0_prog_v);


	#elif SWEET_XBRAID_SPHERE
			SphereData_Spectral t0_prog_phi_pert(sphereDataConfig[0]);
			SphereData_Spectral t0_prog_vrt(sphereDataConfig[0]);
			SphereData_Spectral t0_prog_div(sphereDataConfig[0]);

			SphereData_Physical t0_prog_phi_pert_phys(sphereDataConfig[0]);
			SphereData_Physical t0_prog_vrt_phys(sphereDataConfig[0]);
			SphereData_Physical t0_prog_div_phys(sphereDataConfig[0]);

			t0_prog_phi_pert_phys.physical_update_lambda_array(
						[&](int i, int j, double &io_data)
				{
					io_data = this->simVars->sim.h0 + ((double)braid_Rand())/braid_RAND_MAX;
				}
			);
			t0_prog_vrt_phys.physical_update_lambda_array(
						[&](int i, int j, double &io_data)
				{
					io_data = ((double)braid_Rand())/braid_RAND_MAX;
				}
			);
			t0_prog_div_phys.physical_update_lambda_array(
						[&](int i, int j, double &io_data)
				{
					io_data = ((double)braid_Rand())/braid_RAND_MAX;
				}
			);

			t0_prog_phi_pert.loadSphereDataPhysical(t0_prog_phi_pert_phys);
			t0_prog_vrt.loadSphereDataPhysical(t0_prog_vrt_phys);
			t0_prog_div.loadSphereDataPhysical(t0_prog_div_phys);

			U->data->dataArrays_to_GenericData_SphereData_Spectral(t0_prog_phi_pert, t0_prog_vrt, t0_prog_div);
	#endif
		}
		else
		{
			std::cout << "Setting zero solution " << std::endl;
			/* Sets U as an all zero vector*/
	#if SWEET_XBRAID_SCALAR
			double zero = 0;
			U->data->dataArrays_to_GenericData_Scalar(zero);
	#elif SWEET_XBRAID_PLANE
		#if SWEET_XBRAID_PLANE_SWE
			PlaneData_Spectral t0_prog_h_pert(planeDataConfig[0]);
			t0_prog_h_pert.spectral_set_zero();
		#endif
	
			PlaneData_Spectral t0_prog_u(planeDataConfig[0]);
			t0_prog_u.spectral_set_zero();
	
			PlaneData_Spectral t0_prog_v(planeDataConfig[0]);
			t0_prog_v.spectral_set_zero();

			U->data->dataArrays_to_GenericData_PlaneData_Spectral(
		#if SWEET_XBRAID_PLANE_SWE
										t0_prog_h_pert,
		#endif
										t0_prog_u,
										t0_prog_v);
	
	#elif SWEET_XBRAID_SPHERE
			SphereData_Spectral t0_prog_phi_pert(sphereDataConfig[0]);
			SphereData_Spectral t0_prog_vrt(sphereDataConfig[0]);
			SphereData_Spectral t0_prog_div(sphereDataConfig[0]);

			t0_prog_phi_pert.spectral_set_zero();
			t0_prog_vrt.spectral_set_zero();
			t0_prog_div.spectral_set_zero();

			U->data->dataArrays_to_GenericData_SphereData_Spectral(t0_prog_phi_pert, t0_prog_vrt, t0_prog_div);
	#endif
		}

		*o_U = (braid_Vector) U;

		return 0;
	}



	/* --------------------------------------------------------------------
	 * Create a copy of a vector object.
	 * -------------------------------------------------------------------- */
	braid_Int
	Clone(
			braid_Vector	i_U,
			braid_Vector*	o_V
		)
	{
		sweet_BraidVector* U = (sweet_BraidVector*) i_U;
		sweet_BraidVector* V = create_new_vector(U->level);
		*V = *U;
		*o_V = (braid_Vector) V;

		return 0;
	}


	/* --------------------------------------------------------------------
	 * Destroy vector object.
	 * -------------------------------------------------------------------- */
	braid_Int
	Free(
			braid_Vector	i_U)
	{
		sweet_BraidVector* U = (sweet_BraidVector*) i_U;
		delete U;

		return 0;
	}


	/* --------------------------------------------------------------------
	 * Compute vector sum y = alpha*x + beta*y.
	 * -------------------------------------------------------------------- */
	braid_Int
	Sum(
			double			i_alpha,
			braid_Vector		i_X,
			double			i_beta,
			braid_Vector		io_Y
		)
	{

		sweet_BraidVector* X = (sweet_BraidVector*) i_X;
		sweet_BraidVector* Y = (sweet_BraidVector*) io_Y;

		*Y = *X * i_alpha + *Y * i_beta;

		return 0;
	}


	/* --------------------------------------------------------------------
	 * User access routine to spatial solution vectors and allows for user
	 * output.  The default XBraid parameter of access_level=1, calls 
	 * my_Access only after convergence and at every time point.
	 * -------------------------------------------------------------------- */
	braid_Int
	Access(
				braid_Vector		i_U,
				BraidAccessStatus&	io_astatus
			)
	{
		double     tstart         = (this->tstart);
		double     tstop          = (this->tstop);
		////int        nt             = (this->nt);
	
		double     rnorm, disc_err, t;
		int        iter, level, done, index, myid, it;
		char       filename[255], filename_mesh[255], filename_err[255], filename_sol[255];


		sweet_BraidVector *U = (sweet_BraidVector*) i_U;

		/* Retrieve current time from Status Object */
		////braid_AccessStatusGetT(astatus, &t);
		io_astatus.GetT(&t);
		io_astatus.GetTIndex(&it);
		io_astatus.GetIter(&iter);
		io_astatus.GetLevel(&level);

		/* Retrieve XBraid State Information from Status Object */
		///////////MPI_Comm_rank(app->comm_x, &myid);
		///////////braid_AccessStatusGetTILD(astatus, &t, &iter, &level, &done);
		///////////braid_AccessStatusGetResidual(astatus, &rnorm);


		if(level == 0)
		{


			// Decide whether to output or not
			bool do_output = false;
			double small = 1e-10;

			// output each time step if:
			// output_timestep < 0 (i.e. output every timestep)
			// t == 0
			// t == Tmax
			// t is a multiple of dt_output
			if (
				this->simVars->iodata.output_each_sim_seconds < 0 ||
				std::abs(t) < small ||
				std::abs(t - this->simVars->timecontrol.max_simulation_time) < small ||
				fmod(t, this->simVars->iodata.output_each_sim_seconds) == 0
			)
				do_output = true;

			if (do_output)
				std::cout << "AAA " << t << " " << it << " " <<  t * simVars->iodata.output_time_scale << " " << this->simVars->iodata.output_each_sim_seconds << " " << fmod(t, this->simVars->iodata.output_each_sim_seconds) << " " << do_output << std::endl;

			if (do_output)
			{
				// Output physical solution to file
				if (simVars->xbraid.xbraid_store_iterations)
					this->output_data_file(
								U->data,
								iter,
								it,
								t
					);

				// Compute and store errors w.r.t. ref solution
				if (simVars->xbraid.xbraid_load_ref_csv_files)
				{
					// create containers for ref solution
					if (this->xbraid_data_ref_exact.size() == 0)
					{
						int nt;
						io_astatus.GetNTPoints(&nt);
						for (int i = 0; i < nt + 1; i++)
							this->xbraid_data_ref_exact.push_back(this->create_new_vector(0));
					}

					if (it > 0)
						this->store_parareal_error(
										U->data,
										this->xbraid_data_ref_exact[it]->data,
										N,
										iter /* + 1 */,
										it,
										t,
										this->simVars->xbraid.xbraid_path_ref_csv_files,
										"ref"
						);
				}
				// Compute and store errors w.r.t. fine (serial) solution
				if (simVars->xbraid.xbraid_load_fine_csv_files)
				{
					// create containers for fine solution
					if (this->xbraid_data_fine_exact.size() == 0)
					{
						int nt;
						io_astatus.GetNTPoints(&nt);
						for (int i = 0; i < nt + 1; i++)
							this->xbraid_data_fine_exact.push_back(this->create_new_vector(0));
					}

					if (it > 0)
						this->store_parareal_error(
										U->data,
										this->xbraid_data_fine_exact[it]->data,
										N,
										iter /* + 1 */,
										it,
										t,
										this->simVars->xbraid.xbraid_path_fine_csv_files,
										"fine"
						);
				}
			}

			// Store residual (residual per iteration)
			if (it == 0) {
				double res;
				io_astatus.GetResidual(&res);
				this->output_residual_file(res,
								iter);
			}

		}


		// TODO: verify if convergence stagnates and stop simulation

		return 0;
	}



	/* --------------------------------------------------------------------
	 * Compute norm of a spatial vector 
	 * -------------------------------------------------------------------- */
	braid_Int
	SpatialNorm(
			braid_Vector  i_U,
			double*       o_norm)
	{
		sweet_BraidVector* U = (sweet_BraidVector*) i_U;
		*o_norm = U->data->reduce_norm2();
		return 0;
	}


	/* --------------------------------------------------------------------
	 * Return buffer size needed to pack one spatial braid_Vector.  Here the
	 * vector contains one double at every grid point and thus, the buffer 
	 * size is the number of grid points.
	 * -------------------------------------------------------------------- */
	braid_Int
	BufSize(
			int*			o_size,
			BraidBufferStatus&	o_status)
	{
		*o_size = this->size_buffer;
		return 0;
	}


	/* --------------------------------------------------------------------
	 * Pack a braid_Vector into a buffer.
	 * -------------------------------------------------------------------- */
	braid_Int
	BufPack(
			braid_Vector		i_U,
			void*			o_buffer,
			BraidBufferStatus&	o_status
		)
	{

		sweet_BraidVector* U = (sweet_BraidVector*) i_U;

#if SWEET_XBRAID_SCALAR
		double* dbuffer = (double*) o_buffer;
#else
		std::complex<double>* dbuffer = (std::complex<double>*) o_buffer;
#endif

		U->data->serialize(dbuffer);

		o_status.SetSize( this->size_buffer );
		return 0;
	}

	/* --------------------------------------------------------------------
	 * Unpack a buffer and place into a braid_Vector
	 * -------------------------------------------------------------------- */
	braid_Int
	BufUnpack(
			void*			i_buffer,
			braid_Vector*		o_U,
			BraidBufferStatus&	io_status
		)
	{

		int level = 0;
		///io_status.GetLevel(&level);

		sweet_BraidVector* U = create_new_vector(level);

#if SWEET_XBRAID_SCALAR
		double* dbuffer = (double*) i_buffer;
#else
		std::complex<double>* dbuffer = (std::complex<double>*) i_buffer;
#endif

		U->data->deserialize(dbuffer);

		*o_U = (braid_Vector) U;

		return 0;
	}

};


#endif






