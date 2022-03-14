/*
 * PlaneDataPhysical.hpp
 *
 *  Created on: 14 Mar 2022
 *      Author: Joao Steinstraesser <joao.steinstraesser@usp.br>
 */

#ifndef PLANE_DATA_PHYSICAL_HPP_
#define PLANE_DATA_PHYSICAL_HPP_



#include <sweet/MemBlockAlloc.hpp>
#include <sweet/openmp_helper.hpp>
#include <sweet/plane/PlaneDataConfig.hpp>
#include <sweet/SWEETError.hpp>


#if SWEET_THREADING_SPACE
#define PLANE_DATA_PHYSICAL_FOR_IDX(CORE)				\
		SWEET_THREADING_SPACE_PARALLEL_FOR_SIMD			\
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)	\
		{	CORE;	}

#define PLANE_DATA_PHYSICAL_FOR_IDX_REDUCTION(CORE, REDUCTION)				\
		_Pragma("omp parallel for "##REDUCTION##" "##PROC_BIND_CLOSE##"")	\
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)	\
		{	CORE;	}

#define PLANE_DATA_PHYSICAL_FOR_2D_IDX(CORE)										\
		SWEET_THREADING_SPACE_PARALLEL_FOR_SIMD_COLLAPSE2	\
		for (std::size_t j = 0; j < planeDataConfig->physical_data_size[1]; j++)						\
		{				\
			for (std::size_t i = 0; i < planeDataConfig->physical_data_size[0]; i++)	\
			{			\
				std::size_t idx = j*planeDataConfig->physical_data_size[0]+i;	\
				CORE;	\
			}			\
		}

#else

#define PLANE_DATA_PHYSICAL_FOR_IDX(CORE)				\
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)	\
		{	CORE;	}

#define PLANE_DATA_PHYSICAL_FOR_2D_IDX(CORE)										\
		for (std::size_t j = 0; j < planeDataConfig->physical_data_size[1]; j++)						\
		{				\
			for (std::size_t i = 0; i < planeDataConfig->physical_data_size[0]; i++)	\
			{			\
				std::size_t idx = j*planeDataConfig->physical_data_size[0]+i;	\
				CORE;	\
			}			\
		}

#endif

#if SWEET_THREADING_SPACE
#	include <omp.h>
#endif


class PlaneData_Physical
{

public:
	const PlaneDataConfig *planeDataConfig;

public:
	double *physical_space_data;


	void swap(
			PlaneData_Physical &i_paneData
	)
	{
		assert(planeDataConfig == i_planeData.planeDataConfig);

		std::swap(physical_space_data, i_planeData.physical_space_data);
	}

public:
	PlaneData_Physical(
			const PlaneDataConfig *i_planeDataConfig
	)	:
		/// important: set this to nullptr, since a check for this will be performed by setup(...)
		planeDataConfig(i_planeDataConfig),
		physical_space_data(nullptr)
	{
		alloc_data();
	}


public:
	PlaneData_Physical(
			const PlaneDataConfig *i_planeDataConfig,
			double i_value
	)	:
		/// important: set this to nullptr, since a check for this will be performed by setup(...)
		planeDataConfig(i_planeDataConfig),
		physical_space_data(nullptr)
	{
		alloc_data();
		physical_set_all_value(i_value);
	}


public:
	PlaneData_Physical()	:
		planeDataConfig(nullptr),
		physical_space_data(nullptr)
	{
	}


public:
	PlaneData_Physical(
			const PlaneData_Physical &i_plane_data
	)	:
		planeDataConfig(i_plane_data.planeDataConfig),
		physical_space_data(nullptr)

	{
		if (i_plane_data.planeDataConfig != nullptr)
			alloc_data();

		operator=(i_plane_data);
	}


public:
	PlaneData_Physical(
			PlaneData_Physical &&i_plane_data
	)	:
		planeDataConfig(i_plane_data.planeDataConfig),
		physical_space_data(nullptr)
	{
		if (i_plane_data.planeDataConfig == nullptr)
			return;

		physical_space_data = i_plane_data.physical_space_data;
		i_plane_data.physical_space_data = nullptr;
	}



	/**
	 * Run validation checks to make sure that the physical and spectral spaces match in size
	 */
public:
	inline void check(
			const PlaneDataConfig *i_planeDataConfig
	)	const
	{
		assert(planeDataConfig->physical_res[0] == i_planeDataConfig->physical_res[0]);
		assert(planeDataConfig->physical_res[1] == i_planeDataConfig->physical_res[1]);
	}



public:
	PlaneData_Physical& operator=(
			const PlaneData_Physical &i_plane_data
	)
	{
		if (i_plane_data.planeDataConfig == nullptr)
			return *this;

		if (paneDataConfig == nullptr)
			setup(i_plane_data.planeDataConfig);

		memcpy(physical_space_data, i_plane_data.physical_space_data, sizeof(double)*planeDataConfig->physical_array_data_number_of_elements);

		return *this;
	}


public:
	PlaneData_Physical& operator=(
			PlaneData_Physical &&i_plane_data
	)
	{
		if (planeDataConfig == nullptr)
			setup(i_plane_data.planeDataConfig);

		std::swap(physical_space_data, i_plane_data.physical_space_data);

		return *this;
	}


	PlaneData_Physical operator+(
			const PlaneData_Physical &i_plane_data
	)	const
	{
		check(i_plane_data.planeDataConfig);

		PlaneData_Physical out(planeDataConfig);

		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			out.physical_space_data[idx] = physical_space_data[idx] + i_plane_data.physical_space_data[idx];

		return out;
	}



	PlaneData_Physical& operator+=(
			const PlaneData_Physical &i_plane_data
	)
	{
		check(i_plane_data.planeDataConfig);

		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			physical_space_data[idx] += i_plane_data.physical_space_data[idx];

		return *this;
	}


	PlaneData_Physical& operator+=(
			double i_scalar
	)
	{
		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			physical_space_data[idx] += i_scalar;

		return *this;
	}


	PlaneData_Physical& operator-=(
			const PlaneData_Physical &i_plane_data
	)
	{
		check(i_plane_data.planeDataConfig);

		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			physical_space_data[idx] -= i_plane_data.physical_space_data[idx];

		return *this;
	}


	PlaneData_Physical& operator-=(
			double i_scalar
	)
	{
		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			physical_space_data[idx] -= i_scalar;

		return *this;
	}



	PlaneData_Physical operator-(
			const PlaneData_Physical &i_plane_data
	)	const
	{
		check(i_plane_data.planeDataConfig);

		PlaneData_Physical out(planeDataConfig);

		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			out.physical_space_data[idx] = physical_space_data[idx] - i_plane_data.physical_space_data[idx];

		return out;
	}



	PlaneData_Physical operator-()
	{
		PlaneData_Physical out(planeDataConfig);

		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			out.physical_space_data[idx] = -physical_space_data[idx];

		return out;
	}



	PlaneData_Physical operator*(
			const PlaneData_Physical &i_plane_data
	)	const
	{
		check(i_plane_data.planeDataConfig);

		PlaneData_Physical out(planeDataConfig);

		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t i = 0; i < planeDataConfig->physical_array_data_number_of_elements; i++)
			out.physical_space_data[i] = physical_space_data[i]*i_plane_data.physical_space_data[i];

		return out;
	}



	PlaneData_Physical operator/(
			const PlaneData_Physical &i_plane_data
	)	const
	{
		check(i_plane_data.planeDataConfig);

		check(i_plane_data.planeDataConfig);

		PlaneData_Physical out(planeDataConfig);

		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t i = 0; i < planeDataConfig->physical_array_data_number_of_elements; i++)
			out.physical_space_data[i] = physical_space_data[i]/i_plane_data.physical_space_data[i];

		return out;
	}



	PlaneData_Physical operator*(
			const double i_value
	)	const
	{
		PlaneData_Physical out(planeDataConfig);

		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t i = 0; i < planeDataConfig->physical_array_data_number_of_elements; i++)
			out.physical_space_data[i] = physical_space_data[i]*i_value;

		return out;
	}




	const PlaneData_Physical& operator*=(
			const double i_value
	)	const
	{
		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			physical_space_data[idx] *= i_value;

		return *this;
	}




	PlaneData_Physical operator/(
			double i_value
	)	const
	{
		PlaneData_Physical out(planeDataConfig);

		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			out.physical_space_data[idx] = physical_space_data[idx]/i_value;

		return out;
	}



	PlaneData_Physical operator+(
			double i_value
	)	const
	{
		PlaneData_Physical out(planeDataConfig);

		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			out.physical_space_data[idx] = physical_space_data[idx]+i_value;

		return out;
	}



	PlaneData_Physical operator-(
			double i_value
	)	const
	{
		PlaneData_Physical out(planeDataConfig);

		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			out.physical_space_data[idx] = physical_space_data[idx]-i_value;

		return out;
	}


	PlaneData_Physical operator_scalar_sub_this(
			double i_value
	)	const
	{
		PlaneData_Physical out(planeDataConfig);

		SWEET_THREADING_SPACE_PARALLEL_FOR
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			out.physical_space_data[idx] = i_value - physical_space_data[idx];

		return out;
	}



public:
	void setup(
			const PlaneDataConfig *i_planeDataConfig
	)
	{
		if (planeDataConfig != nullptr)
			SWEETError("Setup called twice!");

		planeDataConfig = i_planeDataConfig;
		alloc_data();
	}



private:
	void alloc_data()
	{
		assert(physical_space_data == nullptr);
		physical_space_data = MemBlockAlloc::alloc<double>(planeDataConfig->physical_array_data_number_of_elements * sizeof(double));
	}




public:
	void setup_if_required(
			const PlaneDataConfig *i_planeDataConfig
	)
	{
		if (planeDataConfig != nullptr)
			return;

		setup(i_planeDataConfig);
	}



public:
	~PlaneData_Physical()
	{
		free();
	}


public:
	void free()
	{
		if (physical_space_data != nullptr)
		{
			MemBlockAlloc::free(physical_space_data, planeDataConfig->physical_array_data_number_of_elements * sizeof(double));
			physical_space_data = nullptr;
		}

		planeDataConfig = nullptr;
	}



////	/*
////	 * Set values for all latitude and longitude degrees
////	 *
////	 * lambda function parameters: (longitude \in [0;2*pi], Gaussian latitude \in [-M_PI/2;M_PI/2])
////	 */
////	void physical_update_lambda(
////			std::function<void(double,double,double&)> i_lambda	///< lambda function to return value for lat/mu
////	)
////	{
////		SWEET_THREADING_SPACE_PARALLEL_FOR
////
////#if SPHERE_DATA_GRID_LAYOUT	== SPHERE_DATA_LAT_CONTINUOUS
////
////		for (int i = 0; i < planeDataConfig->physical_num_lon; i++)
////		{
////			double lon_degree = ((double)i/(double)planeDataConfig->physical_num_lon)*2.0*M_PI;
////
////			for (int j = 0; j < planeDataConfig->physical_num_lat; j++)
////			{
////				//double colatitude = acos(shtns->ct[j]);
////
////				/*
////				 * Colatitude is 0 at the north pole and 180 at the south pole
////				 *
////				 * WARNING: The latitude degrees are not equidistant spaced in the angles!!!! We have to use the shtns->ct lookup table
////				 */
////				//double lat_degree = M_PI*0.5 - colatitude;
////				double lat_degree = planeDataConfig->lat[j];
////
////				i_lambda(lon_degree, lat_degree, physical_space_data[i*planeDataConfig->physical_num_lat + j]);
////			}
////		}
////
////#else
////
////		for (int jlat = 0; jlat < planeDataConfig->physical_num_lat; jlat++)
////		{
////			double lat_degree = planeDataConfig->lat[jlat];
////
////			for (int ilon = 0; ilon < planeDataConfig->physical_num_lon; ilon++)
////			{
////				double lon_degree = ((double)ilon/(double)planeDataConfig->physical_num_lon)*2.0*M_PI;
////
////				//double colatitude = acos(shtns->ct[j]);
////
////				/*
////				 * Colatitude is 0 at the north pole and 180 at the south pole
////				 *
////				 * WARNING: The latitude degrees are not equidistant spaced in the angles!!!! We have to use the shtns->ct lookup table
////				 */
////				//double lat_degree = M_PI*0.5 - colatitude;
////
////				i_lambda(lon_degree, lat_degree, physical_space_data[jlat*planeDataConfig->physical_num_lon + ilon]);
////			}
////		}
////
////#endif
////	}


//////	void physical_update_lambda_array(
//////			std::function<void(int,int,double&)> i_lambda	///< lambda function to return value for lat/mu
//////	)
//////	{
//////
//////		SWEET_THREADING_SPACE_PARALLEL_FOR
//////
//////#if SPHERE_DATA_GRID_LAYOUT	== SPHERE_DATA_LAT_CONTINUOUS
//////
//////		for (int i = 0; i < planeDataConfig->physical_num_lon; i++)
//////		{
//////			for (int j = 0; j < planeDataConfig->physical_num_lat; j++)
//////			{
//////				i_lambda(i, j, physical_space_data[i*planeDataConfig->physical_num_lat + j]);
//////			}
//////		}
//////
//////#else
//////
//////		for (int jlat = 0; jlat < planeDataConfig->physical_num_lat; jlat++)
//////		{
//////			for (int ilon = 0; ilon < planeDataConfig->physical_num_lon; ilon++)
//////			{
//////				i_lambda(ilon, jlat, physical_space_data[jlat*planeDataConfig->physical_num_lon + ilon]);
//////			}
//////		}
//////
//////#endif
//////	}
//////
//////
//////	void physical_update_lambda_array_idx(
//////			std::function<void(int,double&)> i_lambda	///< lambda function to return value for lat/mu
//////	)
//////	{
//////		SWEET_THREADING_SPACE_PARALLEL_FOR
//////
//////		for (std::size_t i = 0; i < planeDataConfig->physical_array_data_number_of_elements; i++)
//////		{
//////			i_lambda(i, physical_space_data[i]);
//////		}
//////	}
//////
//////
//////	/*
//////	 * Set values for all latitude and longitude degrees
//////	 *
//////	 * lambda function parameters: (longitude \in [0;2*pi], Gaussian latitude sin(phi) \in [-1;1])
//////	 */
//////	void physical_update_lambda_gaussian_grid(
//////			std::function<void(double,double,double&)> i_lambda	///< lambda function to return value for lat/mu
//////	)
//////	{
//////		SWEET_THREADING_SPACE_PARALLEL_FOR
//////
//////#if SPHERE_DATA_GRID_LAYOUT	== SPHERE_DATA_LAT_CONTINUOUS
//////
//////		for (int i = 0; i < planeDataConfig->physical_num_lon; i++)
//////		{
//////			double lon_degree = ((double)i/(double)planeDataConfig->physical_num_lon)*2.0*M_PI;
//////
//////			for (int j = 0; j < planeDataConfig->physical_num_lat; j++)
//////			{
//////				double sin_phi = planeDataConfig->lat_gaussian[j];
//////
//////				i_lambda(lon_degree, sin_phi, physical_space_data[i*planeDataConfig->physical_num_lat + j]);
//////			}
//////		}
//////#else
//////
//////		for (int jlat = 0; jlat < planeDataConfig->physical_num_lat; jlat++)
//////		{
//////			double sin_phi = planeDataConfig->lat_gaussian[jlat];
//////
//////			for (int ilon = 0; ilon < planeDataConfig->physical_num_lon; ilon++)
//////			{
//////				double lon_degree = ((double)ilon/(double)planeDataConfig->physical_num_lon)*2.0*M_PI;
//////
//////				i_lambda(lon_degree, sin_phi, physical_space_data[jlat*planeDataConfig->physical_num_lon + ilon]);
//////			}
//////		}
//////#endif
//////	}
//////
//////
//////
//////
//////	/*
//////	 * Set values for all latitude and longitude degrees
//////	 *
//////	 * lambda function parameters:
//////	 *   (longitude \in [0;2*pi], Cogaussian latitude cos(phi) \in [0;1])
//////	 */
//////	void physical_update_lambda_cogaussian_grid(
//////			std::function<void(double,double,double&)> i_lambda	///< lambda function to return value for lat/mu
//////	)
//////	{
//////
//////#if SPHERE_DATA_GRID_LAYOUT	== SPHERE_DATA_LAT_CONTINUOUS
//////
//////		SWEET_THREADING_SPACE_PARALLEL_FOR
//////		for (int i = 0; i < planeDataConfig->physical_num_lon; i++)
//////		{
//////			double lon_degree = (((double)i)/(double)planeDataConfig->physical_num_lon)*2.0*M_PI;
//////
//////			for (int j = 0; j < planeDataConfig->physical_num_lat; j++)
//////			{
//////				double cos_phi = planeDataConfig->lat_cogaussian[j];
//////
//////				/*
//////				 * IDENTITAL FORMULATION
//////				double mu = shtns->ct[j];
//////				double comu = sqrt(1.0-mu*mu);
//////				*/
//////
//////				i_lambda(lon_degree, cos_phi, physical_space_data[i*planeDataConfig->physical_num_lat + j]);
//////			}
//////		}
//////#else
//////
//////		SWEET_THREADING_SPACE_PARALLEL_FOR
//////		for (int jlat = 0; jlat < planeDataConfig->physical_num_lat; jlat++)
//////		{
//////			double cos_phi = planeDataConfig->lat_cogaussian[jlat];
//////
//////			for (int ilon = 0; ilon < planeDataConfig->physical_num_lon; ilon++)
//////			{
//////				double lon_degree = (((double)ilon)/(double)planeDataConfig->physical_num_lon)*2.0*M_PI;
//////
//////				/*
//////				 * IDENTITAL FORMULATION
//////				double mu = shtns->ct[j];
//////				double comu = sqrt(1.0-mu*mu);
//////				*/
//////
//////				i_lambda(lon_degree, cos_phi, physical_space_data[jlat*planeDataConfig->physical_num_lon + ilon]);
//////			}
//////		}
//////#endif
//////	}
//////
//////
//////	void physical_update_lambda_sinphi_grid(
//////			std::function<void(double,double,double&)> i_lambda	///< lambda function to return value for lat/mu
//////	)
//////	{
//////		physical_update_lambda_gaussian_grid(i_lambda);
//////	}
//////
//////	void physical_update_lambda_cosphi_grid(
//////			std::function<void(double,double,double&)> i_lambda	///< lambda function to return value for lat/mu
//////	)
//////	{
//////		physical_update_lambda_cogaussian_grid(i_lambda);
//////	}




	void physical_update_lambda_array_indices(
			std::function<void(int,int,double&)> i_lambda,	///< lambda function to return value for lat/mu
			bool i_anti_aliasing = true
	)
	{
		PLANE_DATA_PHYSICAL_FOR_2D_IDX(
				i_lambda(i, j, physical_space_data[idx])
		);
	}


	void physical_update_lambda_array_idx(
			std::function<void(int,double&)> i_lambda	///< lambda function to return value for lat/mu
	)
	{
		SWEET_THREADING_SPACE_PARALLEL_FOR

		for (std::size_t i = 0; i < planeDataConfig->physical_array_data_number_of_elements; i++)
		{
			i_lambda(i, physical_space_data[i]);
		}
	}


	void physical_update_lambda_unit_coordinates_corner_centered(
			std::function<void(double,double,double&)> i_lambda,	///< lambda function to return value for lat/mu
			bool i_anti_aliasing = true
	)
	{
		PLANE_DATA_PHYSICAL_FOR_2D_IDX(
				i_lambda(
						(double)i/(double)planeDataConfig->physical_res[0],
						(double)j/(double)planeDataConfig->physical_res[1],
						physical_space_data[idx]
				)
		);
	}

	void physical_update_lambda_unit_coordinates_cell_centered(
			std::function<void(double,double,double&)> i_lambda,	///< lambda function to return value for lat/mu
			bool i_anti_aliasing = true
	)
	{
		PLANE_DATA_PHYSICAL_FOR_2D_IDX(
				i_lambda(
						((double)i+0.5)/(double)planeDataConfig->physical_res[0],
						((double)j+0.5)/(double)planeDataConfig->physical_res[1],
						physical_space_data[idx]
				)
		);
	}





	/*
	 * Set all values to zero
	 */
	void physical_set_zero()
	{
		PLANE_DATA_PHYSICAL_FOR_2D_IDX(
				physical_space_data[idx] = 0;
				)
		///SWEET_THREADING_SPACE_PARALLEL_FOR

		///for (int i = 0; i < planeDataConfig->physical_num_lon; i++)
		///	for (int j = 0; j < planeDataConfig->physical_num_lat; j++)
		///		physical_space_data[j*planeDataConfig->physical_num_lon + i] = 0;
	}



	/*
	 * Set all values to a specific value
	 */
	void physical_set_all_value(
			double i_value
	)
	{
		PLANE_DATA_PHYSICAL_FOR_2D_IDX(
				physical_space_data[idx] = i_value;
				)
		////SWEET_THREADING_SPACE_PARALLEL_FOR
		////for (int i = 0; i < planeDataConfig->physical_num_lon; i++)
		////	for (int j = 0; j < planeDataConfig->physical_num_lat; j++)
		////		physical_space_data[j*planeDataConfig->physical_num_lon + i] = i_value;
	}



	/*
	 * Set all values to a specific value
	 */
	void physical_set_value(
			int i_x_idx,
			int i_y_idx,
			double i_value
	)
	{
		physical_space_data[i_y_idx*planeDataConfig->physical_res[0] + i_x_idx] = i_value;
	}



	/**
	 * Return the maximum error norm between this and the given data in physical space
	 */
	double physical_reduce_max(
			const PlaneData_Physical &i_plane_data
	)
	{
		check(i_plane_data.planeDataConfig);

		double error = -1;

		for (std::size_t j = 0; j < planeDataConfig->physical_array_data_number_of_elements; j++)
		{
			error = std::max(
						std::abs(
								physical_space_data[j] - i_plane_data.physical_space_data[j]
							),
							error
						);
		}
		return error;
	}


	double physical_reduce_rms()
	{
		double error = 0;

		for (std::size_t j = 0; j < planeDataConfig->physical_array_data_number_of_elements; j++)
		{
			double &d = physical_space_data[j];
			error += d*d;
		}

		return std::sqrt(error / (double)planeDataConfig->physical_array_data_number_of_elements);
	}



	double physical_reduce_sum()	const
	{
		double sum = 0;
		for (std::size_t j = 0; j < planeDataConfig->physical_array_data_number_of_elements; j++)
			sum += physical_space_data[j];

		return sum;
	}


	/**
	 * return the maximum of all absolute values, use quad precision for reduction
	 */
	double physical_reduce_sum_quad()	const
	{
		double sum = 0;
		double c = 0;
#if SWEET_THREADING_SPACE
#pragma omp parallel for PROC_BIND_CLOSE reduction(+:sum,c)
#endif
		for (std::size_t i = 0; i < planeDataConfig->physical_array_data_number_of_elements; i++)
		{
			double value = physical_space_data[i];

			// Use Kahan summation
			double y = value - c;
			double t = sum + y;
			c = (t - sum) - y;
			sum = t;
		}

		sum -= c;

		return sum;
	}


	/**
	 * return the maximum of all absolute values, use quad precision for reduction
	 */
	double physical_reduce_sum_quad_increasing()	const
	{
		double sum = 0;
		double c = 0;
#if SWEET_THREADING_SPACE
#pragma omp parallel for PROC_BIND_CLOSE reduction(+:sum,c)
#endif
		for (std::size_t i = 0; i < planeDataConfig->physical_array_data_number_of_elements; i++)
		{
			double value = physical_space_data[i]*(double)i;

			// Use Kahan summation
			double y = value - c;
			double t = sum + y;
			c = (t - sum) - y;
			sum = t;
		}

		sum -= c;

		return sum;
	}


	double physical_reduce_sum_metric()
	{
		SWEETError("TODO: Implement metric-scaled summation");
		double sum = 0;
		for (std::size_t j = 0; j < planeDataConfig->physical_array_data_number_of_elements; j++)
		{
			sum += physical_space_data[j];
		}

		return sum;
	}




	/**
	 * Return the maximum error norm between this and the given data in physical space
	 */
	double physical_reduce_max_abs(
			const PlaneData_Physical &i_plane_data
	)	const
	{
		check(i_plane_data.planeDataConfig);

		double error = -1;

		for (std::size_t j = 0; j < planeDataConfig->physical_array_data_number_of_elements; j++)
		{
			error = std::max(
						std::abs(
								physical_space_data[j] - i_plane_data.physical_space_data[j]
							),
							error	// leave the error variable as the 2nd parameter. In case of NaN of the 1st parameter, std::max returns NaN
						);
		}
		return error;
	}



	/**
	 * Return the maximum absolute value
	 */
	double physical_reduce_max_abs()	const
	{
		double error = -1;

		for (std::size_t j = 0; j < planeDataConfig->physical_array_data_number_of_elements; j++)
		{
			error = std::max(
						std::abs(physical_space_data[j]),
						error		// leave the error variable as the 2nd parameter. In case of NaN of the 1st parameter, std::max returns NaN
				);
		}
		return error;
	}


	/**
	 * Return the minimum value
	 */
	double physical_reduce_min()	const
	{
		double error = std::numeric_limits<double>::infinity();

		for (std::size_t j = 0; j < planeDataConfig->physical_array_data_number_of_elements; j++)
			error = std::min(physical_space_data[j], error);

		return error;
	}


	/**
	 * Return the minimum value
	 */
	double physical_reduce_max()	const
	{
		double error = -std::numeric_limits<double>::infinity();

		for (std::size_t j = 0; j < planeDataConfig->physical_array_data_number_of_elements; j++)
			error = std::max(physical_space_data[j], error);

		return error;
	}


	/**
	 * Rescale data so that max_abs returns the given value
	 */
	PlaneData_Physical physical_rescale_to_max_abs(
			double i_new_max_abs
	)	const
	{
		double max_abs = physical_reduce_max_abs();
		double scale = i_new_max_abs/max_abs;

		PlaneData_Physical out(planeDataConfig);

		for (std::size_t j = 0; j < planeDataConfig->physical_array_data_number_of_elements; j++)
			out.physical_space_data[j] = physical_space_data[j]*scale;

		return out;
	}



	bool physical_isAnyNaNorInf()	const
	{
		for (std::size_t i = 0; i < planeDataConfig->physical_array_data_number_of_elements; i++)
		{
			if (std::isnan(physical_space_data[i]) || std::isinf(physical_space_data[i]) != 0)
				return true;
		}

		return false;
	}






	void physical_print(
			int i_precision = -1
	)	const
	{
		if (i_precision >= 0)
			std::cout << std::setprecision(i_precision);

        for (int j = (int)(planeDataConfig->physical_res[1]-1); j >= 0; j--)
        {
        		for (int i = 0; i < planeDataConfig->physical_res[0]; i++)
        		{
        			std::cout << physical_space_data[j*planeDataConfig->physical_res[0]+i];
        			if (i < planeDataConfig->physical_res[0]-1)
        				std::cout << "\t";
        		}
        		std::cout << std::endl;
        }
	}



	void physical_file_write(
			const std::string &i_filename,
			const char *i_title = "",
			int i_precision = 20
	)	const
	{
		std::ofstream file(i_filename, std::ios_base::trunc);

		if (i_precision >= 0)
			file << std::setprecision(i_precision);

		file << "#TI " << i_title << std::endl;
		file << "#TX" << std::endl;
		file << "#TY" << std::endl;

		//file << "lat\\lon\t";
		// Use 0 to make it processable by python
		file << "0\t";

		for (int i = 0; i < planeDataConfig->physical_res[0]; i++)
		{
//			double lon_degree = ((double)i/(double)planeDataConfig->spat_num_lon)*2.0*M_PI;
			double x = ((double)i/(double)planeDataConfig->physical_res[0])*simVars.sim.plane_domain_size[0]; // ????

			file << x;
			if (i < planeDataConfig->physical_res[0]-1)
				file << "\t";
		}
		file << std::endl;

        for (int j = planeDataConfig->physical_res[1]-1; j >= 0; j--)
        {
			double y = ((double)i/(double)planeDataConfig->physical_res[1])*simVars.sim.plane_domain_size[1]; // ????

        		file << y << "\t";

        		for (int i = 0; i < planeDataConfig->physical_res[0]; i++)
        		{
        			file << physical_space_data[j*planeDataConfig->physical_res[0]+i];
        			if (i < planeDataConfig->physical_res[0]-1)
        				file << "\t";
        		}
        		file << std::endl;
        }
        file.close();
	}




	bool physical_file_load(
			const std::string &i_filename,		///< Name of file to load data from
			bool i_binary_data = false	///< load as binary data (disabled per default)
	)
	{
		if (i_binary_data)
		{
			std::ifstream file(i_filename, std::ios::binary);

			if (!file)
				SWEETError(std::string("Failed to open file ")+i_filename);

			file.seekg(0, std::ios::end);
			std::size_t size = file.tellg();
			file.seekg(0, std::ios::beg);


			std::size_t expected_size = sizeof(double)*planeDataConfig->physical_array_data_number_of_elements;

			if (size != expected_size)
			{
				std::cerr << "Error while loading data from file " << i_filename << ":" << std::endl;
				std::cerr << "Size of file " << size << " does not match expected size of " << expected_size << std::endl;
				SWEETError("EXIT");
			}

			if (!file.read((char*)physical_space_data, expected_size))
			{
				std::cerr << "Error while loading data from file " << i_filename << std::endl;
				SWEETError("EXIT");
			}

			return true;
		}


		std::ifstream file(i_filename);
		std::string line;

		bool first_data_line = true;


		/*
		 * set physical data to be valid right here!!!
		 * otherwise it might happen that physical_set_value always requests
		 * data to be converted to physical data, hence overwriting data
		 */

		int row = 0;
		while (row < planeDataConfig->physical_num_lat)
		{
			std::getline(file, line);
			if (!file.good())
			{
				std::cerr << "ERROR: EOF - Failed to read data from file " << i_filename << " in line " << row << std::endl;
				return false;
			}

			// skip comment lines
			if (line[0] == '#')
				continue;

			int last_pos = 0;
			int col = 0;

			if (first_data_line)
			{
				// skip first data line since these are the coordinates
				first_data_line = false;
				continue;
			}

			for (int pos = 0; pos < (int)line.size()+1; pos++)
			{
				if (pos < (int)line.size())
					if (line[pos] != '\t' && line[pos] != ' ')
						continue;

				// skip first element!
				if (last_pos > 0)
				{
					std::string strvalue = line.substr(last_pos, pos-last_pos);
					double i_value = atof(strvalue.c_str());
					int x = col;
					int y = planeDataConfig->physical_res[1]-row-1;

					if (x > planeDataConfig->physical_res[0])
						return false;

					if (y > planeDataConfig->physical_res[1])
						return false;

					physical_set_value(x, y, i_value);

					col++;
				}

				last_pos = pos+1;
		    }

			if (col != planeDataConfig->physical_res[0])
			{
				std::cerr << "ERROR: column mismatch - Failed to read data from file " << i_filename << " in line " << row << ", column " << col << std::endl;
				return false;
			}

			row++;
		}

		if (row != planeDataConfig->physical_res[1])
		{
			std::cerr << "ERROR: rows mismatch - Failed to read data from file " << i_filename << " in line " << row << std::endl;
			return false;
		}


		return true;
	}



	void file_write_raw(
			const std::string &i_filename
	)	const
	{
		std::fstream file(i_filename, std::ios::out | std::ios::binary);
		file.write((const char*)physical_space_data, sizeof(double)*planeDataConfig->physical_array_data_number_of_elements);
	}



	void file_read_raw(
			const std::string &i_filename
	)	const
	{
		std::fstream file(i_filename, std::ios::in | std::ios::binary);
		file.read((char*)physical_space_data, sizeof(double)*planeDataConfig->physical_array_data_number_of_elements);
	}


	void print_debug(
			const char *name
	)	const
	{
		std::cout << name << ":" << std::endl;
		std::cout << "                min: " << this->physical_reduce_min() << std::endl;
		std::cout << "                max: " << this->physical_reduce_max() << std::endl;
		std::cout << "                sum: " << this->physical_reduce_sum() << std::endl;
		std::cout << "                suminc: " << this->physical_reduce_sum_quad_increasing() << std::endl;
		std::cout << std::endl;
	}


	void print()	const
	{
		for (std::size_t idx = 0; idx < planeDataConfig->physical_array_data_number_of_elements; idx++)
			std::cout << physical_space_data[idx] << "\t";
		std::cout << std::endl;
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
PlaneData_Physical operator*(
		const double i_value,
		const PlaneData_Physical &i_array_data
)
{
	return i_array_data*i_value;
}



/**
 * operator to support operations such as:
 *
 * 1.5 - arrayData;
 */
inline
static
PlaneData_Physical operator-(
		const double i_value,
		const PlaneData_Physical &i_array_data
)
{
	return i_array_data.operator_scalar_sub_this(i_value);
}



/**
 * operator to support operations such as:
 *
 * 1.5 + arrayData;
 */
inline
static
PlaneData_Physical operator+(
		const double i_value,
		const PlaneData_Physical &i_array_data
)
{
	return i_array_data+i_value;
}





#endif
