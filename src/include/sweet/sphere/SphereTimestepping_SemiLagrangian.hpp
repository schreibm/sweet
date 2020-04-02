/*
 * SphereDataSemiLangrangian.hpp
 *
 *  Created on: 5 Dec 2015
 *      Author: Martin Schreiber <SchreiberX@gmail.com>
 *
 *  Updated to sphere on 28th March 2018
 */
#ifndef SRC_INCLUDE_SWEET_SPHEREDATASEMILAGRANGIAN_HPP_
#define SRC_INCLUDE_SWEET_SPHEREDATASEMILAGRANGIAN_HPP_

#include <sweet/ScalarDataArray.hpp>
#include <sweet/SWEETMath.hpp>
#include <sweet/sphere/SphereData_Physical.hpp>
#include <sweet/sphere/Convert_ScalarDataArray_to_SphereDataPhysical.hpp>
#include <sweet/SimulationVariables.hpp>

#include <sweet/sphere/Convert_SphereDataPhysical_to_ScalarDataArray.hpp>
#include <sweet/sphere/SphereOperators_Sampler_SphereDataPhysical.hpp>



class SphereTimestepping_SemiLagrangian
{
	SphereOperators_Sampler_SphereDataPhysical sample2D;
	const SphereData_Config *sphereDataConfig;

	const SimulationVariables *simVars;

	SphereData_Physical sl_coriolis;

public:
	SphereTimestepping_SemiLagrangian()	:
		sphereDataConfig(nullptr)
	{
	}


	void setup(
		const SphereData_Config *i_sphereDataConfig,
		const SimulationVariables &i_simVars
	)
	{
		sphereDataConfig = i_sphereDataConfig;
		sample2D.setup(sphereDataConfig);

		simVars = &i_simVars;

		//SphereData_Physical u_lon_
		if (simVars->sim.sphere_use_fsphere)
		{
			FatalError("Not supported");
		}


		sl_coriolis.setup(sphereDataConfig);
		sl_coriolis.physical_update_lambda_gaussian_grid(
			[&](double lon, double mu, double &o_data)
			{
				o_data = mu*2.0*simVars->sim.sphere_rotating_coriolis_omega*simVars->sim.sphere_radius;
			}
		);
	}


	static
	double& alpha()
	{
		static double alpha = 0;
		return alpha;
	}

#if 0
	double u_analytical(double i_lambda, double i_theta)
	{
		double a = 6.37122e6;
		double u0 = (2.0*M_PI*a)/(12.0*24.0*60.0*60.0);

		assert(i_lambda >= 0);
		assert(i_lambda <= 2.0*M_PI);
		assert(i_theta >= -0.5*M_PI);
		assert(i_theta <= 0.5*M_PI);

		return u0*(std::cos(i_theta)*std::cos(alpha()) + std::sin(i_theta)*std::cos(i_lambda)*std::sin(alpha()));
	}


	ScalarDataArray u_analytical(
			const ScalarDataArray &i_lambda,
			const ScalarDataArray &i_theta
	)
	{
		ScalarDataArray ret;
		ret.setup(i_lambda.number_of_elements);
		for (int i = 0; i < (int)i_lambda.number_of_elements; i++)
			ret.scalar_data[i] = u_analytical(i_lambda.scalar_data[i], i_theta.scalar_data[i]);

		return ret;
	}

	double v_analytical(double i_lambda, double i_theta)
	{
		double a = 6.37122e6;
		double u0 = (2.0*M_PI*a)/(12.0*24.0*60.0*60.0);

		assert(i_lambda >= 0);
		assert(i_lambda <= 2.0*M_PI);
		assert(i_theta >= -0.5*M_PI);
		assert(i_theta <= 0.5*M_PI);

		return -u0*std::sin(i_lambda)*std::sin(alpha());
	}


	ScalarDataArray v_analytical(
			const ScalarDataArray &i_lambda,
			const ScalarDataArray &i_theta
	)
	{
		ScalarDataArray ret;
		ret.setup(i_lambda.number_of_elements);
		for (int i = 0; i < (int)i_lambda.number_of_elements; i++)
			ret.scalar_data[i] = v_analytical(i_lambda.scalar_data[i], i_theta.scalar_data[i]);

		return ret;
	}
#endif


	/**
	 * Do 1st order accurate advection on the sphere for given
	 *  - starting point,
	 *  - velocity and
	 *  - timestep size
	 * in Cartesian space 
	 */
	inline static
	void doAdvectionOnSphere(
		const ScalarDataArray &i_pos_x,
		const ScalarDataArray &i_pos_y,
		const ScalarDataArray &i_pos_z,

		const ScalarDataArray &i_vector_x,
		const ScalarDataArray &i_vector_y,
		const ScalarDataArray &i_vector_z,

		ScalarDataArray &o_pos_x,
		ScalarDataArray &o_pos_y,
		ScalarDataArray &o_pos_z,

		double i_approximate_sphere_geometry
	)
	{
		if (i_approximate_sphere_geometry)
		{
			/*
			 * This juse uses an approximation of the sphere geometry.
			 */

			/*
			 * Step 1) Apply the velocity vector in Cartesian space, ignoring the sphere's curvature
			 */
			o_pos_x = i_pos_x + i_vector_x;
			o_pos_y = i_pos_y + i_vector_y;
			o_pos_z = i_pos_z + i_vector_z;

			/*
			 * Step 2) Normalize the resulting position
			 */
			SWEETMath::normalize(o_pos_x, o_pos_y, o_pos_z);
			return;
		}

		/*
		 * This version implements an accurate geometry.
		 */
		{
			/*
			 * Step 1) Compute rotation axis with cross product
			 */
			ScalarDataArray rotation_axis_x(i_pos_x.number_of_elements);
			ScalarDataArray rotation_axis_y(i_pos_x.number_of_elements);
			ScalarDataArray rotation_axis_z(i_pos_x.number_of_elements);

			SWEETMath::cross_prod(
					i_pos_x, i_pos_y, i_pos_z,
					i_vector_x, i_vector_y, i_vector_z,
					rotation_axis_x, rotation_axis_y, rotation_axis_z
				);

			/*
			 * Normalize rotation axis since it's likely not normalized yet
			 */
			SWEETMath::normalize_threshold(
					rotation_axis_x,
					rotation_axis_y,
					rotation_axis_z,
					1e-12				// close-to-0 threshold
				);

			/*
			 * Compute rotation angle
			 * (Figure out why there's no rescaling by 1/(2pi)
			 */
			ScalarDataArray angle = SWEETMath::length(i_vector_x, i_vector_y, i_vector_z);

			/*
			 * Rotate
			 */
			SWEETMath::rotate_3d_normalized_rotation_axis(
					i_pos_x,
					i_pos_y,
					i_pos_z,
					angle,
					rotation_axis_x,
					rotation_axis_y,
					rotation_axis_z,
					o_pos_x,
					o_pos_y,
					o_pos_z
				);
		}
	}

#if 0
	void semi_lag_departure_points_settls(
			const SphereData_Physical &i_u_lon_prev,	// Velocities at time t-1
			const SphereData_Physical &i_v_lat_prev,

			const SphereData_Physical &i_u_lon, 		// Velocities at time t
			const SphereData_Physical &i_v_lat,

			const ScalarDataArray &i_pos_lon_a,		// Position of arrival points lon/lat
			const ScalarDataArray &i_pos_lat_a,

			double i_dt,				///< time step size
			double i_earth_radius,

			ScalarDataArray &o_pos_lon_d, 	///< Position of departure points x / y
			ScalarDataArray &o_pos_lat_d,

			int i_timestepping_order,
			int max_iters = 10,
			double i_convergence_tolerance = 1e-8,
			int i_approximate_sphere_geometry = 0,
			bool use_interpolation_limiters = false,
			bool i_include_coriolis_in_trajectoryxxx = false
	)
	{

		semi_lag_departure_points_settls(
				(i_dt/i_earth_radius)*i_u_lon_prev,	// Velocities at time t-1
				(i_dt/i_earth_radius)*i_v_lat_prev,

				(i_dt/i_earth_radius)*i_u_lon, 		// Velocities at time t
				(i_dt/i_earth_radius)*i_v_lat,

				i_pos_lon_a,		// Position of arrival points lon/lat
				i_pos_lat_a,

				o_pos_lon_d, 	///< Position of departure points x / y
				o_pos_lat_d,

				i_timestepping_order,
				max_iters,
				i_convergence_tolerance,
				i_approximate_sphere_geometry,
				use_interpolation_limiters,
				i_include_coriolis_in_trajectoryxxx
		);
	}
#endif


	void semi_lag_departure_points_settls(
			const SphereData_Physical &i_u_lon_prev,	// Velocities at time t-1
			const SphereData_Physical &i_v_lat_prev,

			const SphereData_Physical &i_u_lon, 		// Velocities at time t
			const SphereData_Physical &i_v_lat,

			const ScalarDataArray &i_pos_lon_a,		// Position of arrival points lon/lat
			const ScalarDataArray &i_pos_lat_a,

			ScalarDataArray &o_pos_lon_d, 	///< Position of departure points x / y
			ScalarDataArray &o_pos_lat_d,

			int i_timestepping_order,
			int max_iters = 10,
			double i_convergence_tolerance = 1e-8,
			int i_approximate_sphere_geometry = 0,
			bool use_interpolation_limiters = false,
			bool i_include_coriolis_in_trajectoryxxx = false
	)
	{
		std::size_t num_elements = i_pos_lon_a.number_of_elements;

		if (i_timestepping_order == 1)
		{
			/**
			 * TODO: This could be done by the setup phase
			 */
			ScalarDataArray pos_x_a(num_elements);
			ScalarDataArray pos_y_a(num_elements);
			ScalarDataArray pos_z_a(num_elements);

			// polar => Cartesian coordinates
			SWEETMath::latlon_to_cartesian(
					i_pos_lon_a, i_pos_lat_a,
					pos_x_a, pos_y_a, pos_z_a
				);

			ScalarDataArray u_lon_array = Convert_SphereDataPhysical_to_ScalarDataArray::physical_convert(i_u_lon);
			ScalarDataArray v_lat_array = Convert_SphereDataPhysical_to_ScalarDataArray::physical_convert(i_v_lat);

			ScalarDataArray vel_x(num_elements);
			ScalarDataArray vel_y(num_elements);
			ScalarDataArray vel_z(num_elements);

			// polar => Cartesian coordinates
			SWEETMath::latlon_velocity_to_cartesian_velocity(
					i_pos_lon_a, i_pos_lat_a,
					u_lon_array, v_lat_array,
					&vel_x, &vel_y, &vel_z
				);

			// go to departure point
			ScalarDataArray pos_x_d = pos_x_a - vel_x;
			ScalarDataArray pos_y_d = pos_y_a - vel_y;
			ScalarDataArray pos_z_d = pos_z_a - vel_z;

			// normalize
			ScalarDataArray norm = (pos_x_d*pos_x_d + pos_y_d*pos_y_d + pos_z_d*pos_z_d).inv_sqrt();

			pos_x_d *= norm;
			pos_y_d *= norm;
			pos_z_d *= norm;

			SWEETMath::cartesian_to_latlon(pos_x_d, pos_y_d, pos_z_d, o_pos_lon_d, o_pos_lat_d);
			return;
		}

		if (i_timestepping_order == 2)
		{
			/**
			 * See SETTLS paper
			 * Hortal, M. (2002). The development and testing of a new two-time-level semi-Lagrangian scheme (SETTLS) in the ECMWF forecast model. Q. J. R. Meteorol. Soc., 2, 1671–1687.
			 * 
			 * We use the SETTLS formulation also to compute the departure points.
			 */

			// Extrapolate velocities at departure points
			SphereData_Physical u_extrapol = 2.0*i_u_lon - i_u_lon_prev;
			SphereData_Physical v_extrapol = 2.0*i_v_lat - i_v_lat_prev;


			/**
			 * TODO: This could be done by the setup phase
			 */
			// Compute Cartesian arrival points
			ScalarDataArray pos_x_a(num_elements);
			ScalarDataArray pos_y_a(num_elements);
			ScalarDataArray pos_z_a(num_elements);
			SWEETMath::latlon_to_cartesian(
					i_pos_lon_a, i_pos_lat_a,
					pos_x_a, pos_y_a, pos_z_a
				);

			// Convert velocities along lon/lat to scalardata array
			ScalarDataArray u_lon_array;
			ScalarDataArray v_lat_array;

			u_lon_array = Convert_SphereDataPhysical_to_ScalarDataArray::physical_convert(i_u_lon);
			v_lat_array = Convert_SphereDataPhysical_to_ScalarDataArray::physical_convert(i_v_lat);

			// Compute Cartesian velocities
			ScalarDataArray vel_x(num_elements);
			ScalarDataArray vel_y(num_elements);
			ScalarDataArray vel_z(num_elements);

			// Polar => Cartesian coordinates
			SWEETMath::latlon_velocity_to_cartesian_velocity(
					i_pos_lon_a, i_pos_lat_a,
					u_lon_array, v_lat_array,
					&vel_x, &vel_y, &vel_z
				);

			/*
			 * Setup iterations
			 */
			// Departure points for iterations
			ScalarDataArray pos_x_d = pos_x_a;
			ScalarDataArray pos_y_d = pos_y_a;
			ScalarDataArray pos_z_d = pos_z_a;


			double diff = 999;
			int iters = 0;
			for (; iters < max_iters; iters++)
			{
				SWEETMath::cartesian_to_latlon(pos_x_d, pos_y_d, pos_z_d, o_pos_lon_d, o_pos_lat_d);

#if SWEET_DEBUG
				bool stop = false;
				if (pos_x_d.reduce_isAnyNaNorInf())
				{
					std::cout << "iters: " << iters << std::endl;
					std::cout << "NaN/Inf in pos_x_d" << std::endl;
					stop = true;
				}

				if (pos_y_d.reduce_isAnyNaNorInf())
				{
					std::cout << "iters: " << iters << std::endl;
					std::cout << "NaN/Inf in pos_y_d" << std::endl;
					stop = true;
				}

				if (pos_z_d.reduce_isAnyNaNorInf())
				{
					std::cout << "iters: " << iters << std::endl;
					std::cout << "NaN/Inf in pos_z_d" << std::endl;
					stop = true;
				}


				if (o_pos_lon_d.reduce_isAnyNaNorInf())
				{
					std::cout << "iters: " << iters << std::endl;
					std::cout << "NaN/Inf in o_pos_lon_d" << std::endl;
					stop = true;
				}

				if (o_pos_lat_d.reduce_isAnyNaNorInf())
				{
					std::cout << "iters: " << iters << std::endl;
					std::cout << "NaN/Inf in o_pos_lat_d" << std::endl;
					stop = true;
				}

				if (stop)
				{
					FatalError("STOP");
				}
#endif


				ScalarDataArray u_lon_extrapol = sample2D.bilinear_scalar(u_extrapol, o_pos_lon_d, o_pos_lat_d, true);
				ScalarDataArray v_lat_extrapol = sample2D.bilinear_scalar(v_extrapol, o_pos_lon_d, o_pos_lat_d, true);


				// convert extrapolated velocities to Cartesian velocities
				ScalarDataArray vel_x_extrapol(num_elements);
				ScalarDataArray vel_y_extrapol(num_elements);
				ScalarDataArray vel_z_extrapol(num_elements);

				// polar => Cartesian coordinates
				SWEETMath::latlon_velocity_to_cartesian_velocity(
						o_pos_lon_d, o_pos_lat_d,
						u_lon_extrapol, v_lat_extrapol,
						&vel_x_extrapol, &vel_y_extrapol, &vel_z_extrapol
					);


				ScalarDataArray new_pos_x_d(num_elements);
				ScalarDataArray new_pos_y_d(num_elements);
				ScalarDataArray new_pos_z_d(num_elements);

				doAdvectionOnSphere(
					pos_x_a,
					pos_y_a,
					pos_z_a,

					-0.5*(vel_x_extrapol + vel_x),
					-0.5*(vel_y_extrapol + vel_y),
					-0.5*(vel_z_extrapol + vel_z),

					new_pos_x_d,
					new_pos_y_d,
					new_pos_z_d,

					i_approximate_sphere_geometry
				);

				if (i_convergence_tolerance > 0)
				{
					diff =  (pos_x_d-new_pos_x_d).reduce_maxAbs() +
							(pos_y_d-new_pos_y_d).reduce_maxAbs() +
							(pos_z_d-new_pos_z_d).reduce_maxAbs();

					pos_x_d = new_pos_x_d;
					pos_y_d = new_pos_y_d;
					pos_z_d = new_pos_z_d;

					if (diff < i_convergence_tolerance)
					   break;
				}
			}

			if (i_convergence_tolerance > 0)
			{
				if (diff > i_convergence_tolerance)
				{
					std::cout << "WARNING: Over convergence tolerance" << std::endl;
					std::cout << "+ Iterations: " << iters << std::endl;
					std::cout << "+ maxAbs: " << diff << std::endl;
					std::cout << "+ Convergence tolerance: " << i_convergence_tolerance << std::endl;
				}
			}

			// convert final points from Cartesian space to angular space
			SWEETMath::cartesian_to_latlon(pos_x_d, pos_y_d, pos_z_d, o_pos_lon_d, o_pos_lat_d);
			return;
		}

		FatalError("Only 1st and 2nd order time integration supported");
	}

};

#endif /* SRC_INCLUDE_SWEET_SPHEREDATASEMILAGRANGIAN_HPP_ */
