//#if !SWEET_USE_PLANE_SPECTRAL_SPACE
//	#error "Spectral space not activated"
//#endif

#if SWEET_GUI
#	error	"GUI not supported"
#endif

#include "../include/sweet/plane/PlaneData.hpp"
#include <sweet/SimulationVariables.hpp>
#include "../include/sweet/plane/PlaneOperators.hpp"

#include <math.h>
#include <ostream>
#include <sstream>
#include <unistd.h>
#include <iomanip>
#include <stdio.h>
#include <cassert>

// Plane data config
PlaneDataConfig planeDataConfigInstance;
PlaneDataConfig *planeDataConfig = &planeDataConfigInstance;

SimulationVariables simVars;

#if SWEET_DEBUG
#include <fenv.h>
static void __attribute__ ((constructor))
trapfpe ()
{
	/* Enable some exceptions.  At startup all exceptions are masked.  */

	feenableexcept (FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
}
#endif

int main(int i_argc, char *i_argv[])
{
#if SWEET_DEBUG
	trapfpe();
#endif

	// override flag
	SimulationVariables simVars;
	simVars.disc.use_spectral_basis_diffs = true;

	if (!simVars.setupFromMainParameters(i_argc, i_argv))
		return -1;

	if (simVars.disc.use_spectral_basis_diffs)
		std::cout << "Using spectral diffs" << std::endl;
	else
		std::cout << "Using kernel-based diffs" << std::endl;

	double prev_error_diff_x = 0;
	double prev_error_diff_y = 0;

	double prev_error_diff2_x = 0;
	double prev_error_diff2_y = 0;

	double prev_error_lap = 0;
	double prev_error_bilap = 0;

	/*
	 * iterate over resolutions, starting by res[0] given e.g. by program parameter -n
	 */
	std::size_t res_x = simVars.disc.res_physical[0];
	std::size_t res_y = simVars.disc.res_physical[1];

	std::size_t max_res = 2048;

	if (res_x > max_res || res_y > max_res)
		max_res = std::max(res_x, res_y);

	for (; res_x <= max_res && res_y <= max_res; res_x *= 2, res_y *= 2)
	{
		double tolerance_increase = sqrt(res_x) + sqrt(res_y);

		double max_aspect = simVars.sim.domain_size[0] / simVars.sim.domain_size[1];
		if (max_aspect < 1.0)
			max_aspect = 1.0 / max_aspect;

		tolerance_increase *= max_aspect;

		/*
		 * error tolerance for machine accuracy
		 *
		 * We assume 1e-12 for double precision
		 */
		double eps = 1e-9 * tolerance_increase;

		/*
		 * error tolerance for convergence
		 *
		 * Here, we are very patronizing due to flickering convergence for coarse solutions which
		 * are not really representable in the Fouerier space where the discretization errors
		 * are dominating.
		 */
		double eps_convergence = 1e-4 * tolerance_increase;

		std::cout << "*************************************************************" << std::endl;
		std::cout << "Testing operators with resolution " << res_x << " x " << res_y << std::endl;
		std::cout << "*************************************************************" << std::endl;
		std::size_t res[2] =
		{ res_x, res_y };

		simVars.disc.res_physical[0] = res[0];
		simVars.disc.res_physical[1] = res[1];

		simVars.disc.res_spectral[0] = 0;
		simVars.disc.res_spectral[1] = 0;

		simVars.reset();

		planeDataConfigInstance.setupAuto(simVars.disc.res_physical, simVars.disc.res_spectral, simVars.misc.reuse_spectral_transformation_plans);

		/*
		 * keep h in the outer regions to allocate it only once and avoid reinitialization of FFTW
		 */
		PlaneData h(planeDataConfig);

		{
			std::cout << "**********************************************" << std::endl;
			std::cout << "> Physical resolution (" << simVars.disc.res_physical[0] << "x" << simVars.disc.res_physical[1] << ")" << std::endl;
			std::cout << "> Spectral resolution (" << simVars.disc.res_spectral[0] << "x" << simVars.disc.res_spectral[1] << ")" << std::endl;
			std::cout << "> Domain size (" << simVars.sim.domain_size[0] << "x" << simVars.sim.domain_size[1] << ")" << std::endl;
			std::cout << "**********************************************" << std::endl;
			std::cout << "error tol = " << eps << std::endl;
			std::cout << "**********************************************" << std::endl;

			/**
			 * 2nd order differential operators on high mode functions (nyquist frequency)
			 *
			 * note, that the function on which the 2nd diff operator is computed on has
			 * to be scaled up be a factor of domain_size^2, since e.g.
			 *
			 * diff(sin(2 pi x / size), x, x) = 4.0 pi^2 sin(2 pi x / size) / size^2
			 */
			{
				std::cout << std::endl;
				std::cout << " Testing differentiation for different frequencies (including Nyquist)" << std::endl;

				PlaneData h_diff_x(planeDataConfig);
				PlaneData h_diff_y(planeDataConfig);
				PlaneData h_diff2_x(planeDataConfig);
				PlaneData h_diff2_y(planeDataConfig);
				PlaneData h_bilaplace(planeDataConfig);

				PlaneOperators op(planeDataConfig, simVars.sim.domain_size, simVars.disc.use_spectral_basis_diffs);

				double freq_x = 0;
				double freq_y = 0;

				//Nyquist freq
				std::size_t nyq = simVars.disc.res_physical[0] / 2;

				//Vary frequencies
				for (std::size_t k = 0; k <= 4; k++)
				{
					if (k == 0) //Fix a given frequency
					{
						freq_x = 5;
						freq_y = 5;
					}
					else
					{    // Vary with k
						freq_x = ((double)k * (double)nyq) / 4.0;
						freq_y = ((double)k * (double)nyq) / 4.0;
					}

					double fx = 2.0 * freq_x * M_PIl;
					double fy = 2.0 * freq_y * M_PIl;

					for (int j = 0; j < simVars.disc.res_physical[1]; j++)
					{
						for (int i = 0; i < simVars.disc.res_physical[0]; i++)
						{
							double x = (((double)i + 0.5) / (double)simVars.disc.res_physical[0]); //*simVars.sim.domain_size[0];
							double y = (((double)j + 0.5) / (double)simVars.disc.res_physical[1]); //*simVars.sim.domain_size[1];

							double sin_x = sin(fx * x);
							double cos_x = cos(fx * x);
							double sin_y = sin(fy * y);
							double cos_y = cos(fy * y);

							double dx = simVars.sim.domain_size[0];
							double dy = simVars.sim.domain_size[1];

							h.p_physical_set(j, i, sin_x * sin_y);

							double diff_x = fx * cos_x * sin_y / (simVars.sim.domain_size[0]);
							double diff_y = fy * sin_x * cos_y / (simVars.sim.domain_size[1]);

							h_diff_x.p_physical_set(j, i, diff_x);
							h_diff_y.p_physical_set(j, i, diff_y);

							h_diff2_x.p_physical_set(j, i, -fx * fx * sin_x * sin_y / (dx * dx));
							h_diff2_y.p_physical_set(j, i, -fy * fy * sin_x * sin_y / (dy * dy));

							h_bilaplace.p_physical_set(j, i,
							// d/dx
									fx * fx * fx * fx * sin_x * sin_y / (dx * dx * dx * dx) + fx * fx * fy * fy * sin_x * sin_y / (dy * dy * dx * dx)
									// d/dy
											+ fy * fy * fx * fx * sin_x * sin_y / (dx * dx * dy * dy) + fy * fy * fy * fy * sin_x * sin_y / (dy * dy * dy * dy));
						}
					}

					//This assumes freq_x = freq_y
					//h_bilaplace=8.0*freq_x*freq_x*M_PIl*M_PIl*8.0*freq_x*freq_x*M_PIl*M_PIl*h;

					// Normalization of errors
					double norm_fx = fx / simVars.sim.domain_size[0];
					double norm_fy = fy / simVars.sim.domain_size[1];

					// Also take into account the errors of FFT
					double norm_fft_x = std::sqrt(simVars.disc.res_physical[0]);
					double norm_fft_y = std::sqrt(simVars.disc.res_physical[1]);

					double err_x = (op.diff_c_x(h) - h_diff_x).reduce_maxAbs() / norm_fx / norm_fft_x;
					double err_y = (op.diff_c_y(h) - h_diff_y).reduce_maxAbs() / norm_fy / norm_fft_y;

					// diff2 normalization = 4.0 pi^2 / L^2
					double err2_x = (op.diff2_c_x(h) - h_diff2_x).reduce_maxAbs() / (norm_fx * norm_fx) / norm_fft_x;
					double err2_y = (op.diff2_c_y(h) - h_diff2_y).reduce_maxAbs() / (norm_fy * norm_fy) / norm_fft_y;

					double err_laplace = (op.laplace(h) - h_diff2_x - h_diff2_y).reduce_maxAbs()
							/ (norm_fx * norm_fx + norm_fy * norm_fy)
							/ (norm_fft_x + norm_fft_y);

					double err_bilaplace = (op.laplace(op.laplace(h)) - h_bilaplace).reduce_maxAbs()
							/ (norm_fx * norm_fx * norm_fx * norm_fx + norm_fy * norm_fy * norm_fy * norm_fy)
							/ (norm_fft_x + norm_fft_y)	// for first laplace operator
							/ (norm_fft_x + norm_fft_y) // for second laplace operator
							;

					if (simVars.disc.use_spectral_basis_diffs)
					{
						std::cout << "frequency = " << freq_x << " of " << simVars.disc.res_physical[0] / 2 << std::endl;
						std::cout << " + error diff x = " << err_x << std::endl;
						std::cout << " + error diff y = " << err_y << std::endl;
						std::cout << " + error diff2 x = " << err2_x << std::endl;
						std::cout << " + error diff2 y = " << err2_y << std::endl;
						std::cout << " + error laplace = " << err_laplace << std::endl;
						std::cout << " + error bilaplace = " << err_bilaplace << std::endl;

						if (std::max({ err_x, err_y, err2_x, err2_y, err_laplace, err_bilaplace }) > eps)
							FatalError("SPEC: Error threshold for diff operators too high for spectral differentiation!");
					}
					else
					{
						std::cout << "Tests skipped: Test without spectral derivatives are not applicable here!" << std::endl;

#if 0
						// THESE TESTS ARE NOW DEACTIVATED
						if(k==0)
						{
							double conv_x = prev_error_diff_x/err_x;
							double conv_y = prev_error_diff_y/err_y;
							double conv2_x = prev_error_diff2_x/err2_x;
							double conv2_y = prev_error_diff2_y/err2_y;
							double conv_lap = prev_error_lap/err_laplace;
							double conv_bilap = prev_error_bilap/err_bilaplace;
							std::cout << "frequency x = " << freq_x << " of " << simVars.disc.res_physical[0]/2 << std::endl;
							std::cout << "frequency y = " << freq_y << " of " << simVars.disc.res_physical[1]/2 << std::endl;
							std::cout << "error diff x = " << err_x << std::endl;
							std::cout << "error diff y = " << err_y << std::endl;
							std::cout << "error diff2 x = " << err2_x << std::endl;
							std::cout << "error diff2 y = " << err2_y << std::endl;
							std::cout << "error laplacian = " << err_laplace<< std::endl;
							std::cout << "error bilaplacian = " << err_bilaplace<< std::endl;
							std::cout << "conv x = " << conv_x << std::endl;
							std::cout << "conv y = " << conv_y << std::endl;
							std::cout << "conv2 x = " << conv2_x << std::endl;
							std::cout << "conv2 y = " << conv2_y << std::endl;
							std::cout << "conv lap = " << conv_lap << std::endl;
							std::cout << "conv bilap = " << conv_bilap << std::endl;

							if (std::min(std::abs(
													{	conv_x, conv_y, conv2_x, conv2_y, conv_lap, conv_bilap})) != 0)
							{
								if (std::max(
												{	std::abs(conv_x-4.0), std::abs(conv_y-4.0), std::abs(conv2_x-4.0), std::abs(conv2_y-4.0), std::abs(conv_lap-4.0), std::abs(conv_bilap-4.0)}) > eps_convergence)
								{
									std::cerr << "Cart: Error threshold exceeded, no convergence given!" << std::endl;
									exit(-1);
								}
							}
							prev_error_diff_x = err_x;
							prev_error_diff_y = err_y;
							prev_error_diff2_x = err2_x;
							prev_error_diff2_y = err2_y;
							prev_error_lap=err_laplace;
							prev_error_bilap=err_bilaplace;
							break;

						}
#endif
					}
				}
			}

			std::cout << "TEST A: DONE" << std::endl;


			/*
			 * Test * operator and anti-aliasing
			 */
#if !SWEET_USE_PLANE_SPECTRAL_DEALIASING && 0

			std::cout << "Skipping dealiasing tests since SWEET is compiled without anti-aliasing" << std::endl;

#else

			{
				std::cout << "----------------------------------------" << std::endl;
				std::cout << " Testing multiplication and de-aliasing" << std::endl;
				std::cout << "----------------------------------------" << std::endl;

				PlaneData h1(planeDataConfig);
				PlaneData h2(planeDataConfig);
				PlaneData h12_analytical(planeDataConfig);
				PlaneData h12_dealiased(planeDataConfig);
				PlaneData h12_noalias(planeDataConfig);
				PlaneData h12_truncated(planeDataConfig);

				PlaneOperators op(planeDataConfig, simVars.sim.domain_size, simVars.disc.use_spectral_basis_diffs);

				// Nyquist freq in physical space
				int physical_nyq_freq = simVars.disc.res_physical[0] / 2;
				std::cout << "> Nyquist frequency: " << physical_nyq_freq << std::endl;

				// Truncated Nyquist freq in spectral space
				int spectral_nyq_trunc_freq = 2 * physical_nyq_freq / 3;
				std::cout << "> Truncated Nyquist frequency: " << spectral_nyq_trunc_freq << std::endl;
				std::cout << "> Spectral space modes[0] / 2: " << simVars.disc.res_spectral[0]/2 << std::endl;

#if SWEET_USE_PLANE_SPECTRAL_DEALIASING
				if (spectral_nyq_trunc_freq != simVars.disc.res_spectral[0]/2)
					FatalError("Inconsistent effective Nyquist frequency!");
#endif

				std::cout << std::endl;

				// Total num of freqs
				int n = simVars.disc.res_physical[0];

				// dx, dy
				double dx = 1.0 / simVars.disc.res_physical[0];
				double dy = 1.0 / simVars.disc.res_physical[1];

				for (std::size_t k = 0; k < 7; k++)
				{
					// We test with two different frequencies for the tests

					int freq_1 = physical_nyq_freq / 4.0 + k * physical_nyq_freq / 8.0; // 2 dx wave

					// double freq_1 = 2*nyq_freq-freq_2-1;2*k;
					int freq_2 = physical_nyq_freq / 4.0; // 10 dx wave

					// Important: Make sure freq_1 >= freq_2 always !!!!!!!!!!!
					assert(freq_1 >= freq_2);

					// Product frequencies
					int freq_sum = freq_1 + freq_2;
					int freq_sub = freq_1 - freq_2;
					assert(freq_sum > freq_sub);

					// Frequency info
					std::cout << "freq_1 = " << freq_1 << ", freq_2 = " << freq_2 << std::endl;
					std::cout << "freq_sum = " << freq_sum << ", freq_sub = " << freq_sub << std::endl;

					/*
					 * Aliasing on original (physical) spectrum
					 */
					double physical_trunc_sum = 1.0;
					bool is_alias_present = false;
					if (freq_sum > (double)physical_nyq_freq) //these modes cannot be represented on the grid, and will be aliased
					{
						physical_trunc_sum = 0.0;
						is_alias_present = true;
					}

					if (is_alias_present)
					{
						std::cout << " + Frequency " << freq_sum << " is not representable on this grid and will contaminate the mode " << n - freq_sum
								<< " (alias on low frequency)" << std::endl;
					}
					else
					{
						std::cout << " + Frequency " << freq_sum << " will not produce an aliasing on original multiplication spectrum" << std::endl;
					}

					/*
					 * Aliasing on truncated (spectral) spectrum
					 */
					int spectral_trunc_freq1 = 1;
					int spectral_trunc_freq2 = 1;
					int spectral_trunc_freq_sum = 1;
					bool is_alias_trunc_present = false;
					bool is_alias_trunc_multiplication_present = false;

					if (freq_1 > spectral_nyq_trunc_freq) //these modes cannot be represented on the truncated spectrum
					{
						spectral_trunc_freq1 = 0;
						is_alias_trunc_present = true;
						std::cout << " + Frequency " << freq_1 << " is not representable on the truncated spectrum so multiplication will be zero for truncated spectrum" << std::endl;
					}
					if (freq_2 > spectral_nyq_trunc_freq) //these modes cannot be represented on the truncated spectrum
					{
						spectral_trunc_freq2 = 0;
						is_alias_trunc_present = true;
						std::cout << " + Frequency " << freq_2 << " is not representable on the truncated spectrum so multiplication will be zero for truncated spectrum" << std::endl;
					}
					if (freq_sum > spectral_nyq_trunc_freq) //these modes cannot be represented on the truncated spectrum
					{
						spectral_trunc_freq_sum = 0;
						is_alias_trunc_multiplication_present = true;
						std::cout << " + Frequency " << freq_sum << " is not representable on the truncated spectrum so multiplication will truncate this high mode" << std::endl;
					}
					if (!is_alias_trunc_present && !is_alias_trunc_multiplication_present)
						std::cout << " + Frequency " << freq_sum << " will not introduce an aliasing on truncated multiplication spectrum" << std::endl;

					// cos(a x) cos(b x)  = 1/2 (cos( (a-b) x) + cos( (a+b) x))
					for (int j = 0; j < simVars.disc.res_physical[1]; j++)
					{
						for (int i = 0; i < simVars.disc.res_physical[0]; i++)
						{
							double x = (double)i * dx;
							double y = (double)j * dy;
							h1.p_physical_set(j, i, cos(2.0 * freq_1 * M_PIl * x) * cos(2.0 * freq_1 * M_PIl * y));
							h2.p_physical_set(j, i, cos(2.0 * freq_2 * M_PIl * x) * cos(2.0 * freq_2 * M_PIl * y));

							/*
							 * Analytical solution
							 * Doesn't care about resolution
							 */
							h12_analytical.p_physical_set(j, i,
									0.5 * (cos(2.0 * (freq_sub) * M_PIl * x) + cos(2.0 * (freq_sum) * M_PIl * x))
									* 0.5 * (cos(2.0 * (freq_sub) * M_PIl * y) + cos(2.0 * (freq_sum) * M_PIl * y)));

							/*
							 * Solution without aliasing.
							 * This is how SWEET should behave.
							 *
							 * "trunc_sum" controls if the frequency is included or not
							 */
							h12_noalias.p_physical_set(j, i,
									0.5 * (cos(2.0 * (freq_sub) * M_PIl * x) + physical_trunc_sum * cos(2.0 * (freq_sum) * M_PIl * x)) * 0.5
											* (cos(2.0 * (freq_sub) * M_PIl * y) + physical_trunc_sum * cos(2.0 * (freq_sum) * M_PIl * y)));

							h12_truncated.p_physical_set(j, i,
									spectral_trunc_freq1 * spectral_trunc_freq2 * 0.5 * (cos(2.0 * (freq_sub) * M_PIl * x) + spectral_trunc_freq_sum * cos(2.0 * (freq_sum) * M_PIl * x)) * 0.5
											* (cos(2.0 * (freq_sub) * M_PIl * y) + spectral_trunc_freq_sum * cos(2.0 * (freq_sum) * M_PIl * y)));
						}
					}

					// Standard multiplication
					// Iff there's no aliasing possible, this should return the correct solution
					double err_mult = (h1 * h2 - h12_analytical).reduce_maxAbs();

					// Multiplication with dealiasing from * operator
					// Even if there's dealiasing, this should return the non-aliased result
					double err_mult_dealias = (h1 * h2 - h12_noalias).reduce_maxAbs();

					//Multiplication with dealiasing from mult function (truncation)
					/// TODO: CHECK THIS
					/// TODO: CHECK THIS
					/// TODO: CHECK THIS
					//double err_mult_dealias2 = (h1.mult(h2)-h12_truncated).reduce_maxAbs();
					double err_mult_dealias2 = 0;

#if 0
					std::cout << "error mult * with possibly aliased exact solution = " << err_mult << std::endl;
					std::cout << "error mult * with respect to dealised exact solution = " << err_mult_dealias << std::endl;
					std::cout << "error mult function with respect to truncated and dealiased exact solution = " << err_mult_dealias2 << std::endl;
#endif

#if SWEET_USE_PLANE_SPECTRAL_SPACE && SWEET_USE_PLANE_SPECTRAL_DEALIASING
					if (!is_alias_trunc_multiplication_present)
					{
						if (err_mult > eps)
						{
							std::cout << "ERROR" << std::endl;
							std::cout << " + err_mult: " << err_mult << std::endl;
							std::cout << " + eps: " << eps << std::endl;
							FatalError("No aliasing present, but error significantly high");
						}
					}

					/**
					 * Check correct dealiasing.
					 * If error is too high, dealiasing obviously failed
					 */
					if (err_mult_dealias > eps)
					{
						std::cout << "error operator*(...) with respect to dealised exact solution = " << err_mult_dealias << std::endl;
//						std::cout << "error operator*(...) with possibly aliased exact solution = " << err_mult << std::endl;
//						std::cout << "error mult() function with respect to truncated and dealiased exact solution = " << err_mult_dealias2 << std::endl;

						std::cerr << " WARNING: threshold for multiplication * operator too high !" << std::endl;
						if (is_alias_present)
							std::cerr << "    Multiplication has alias but dealiasing not able to remove it or removed it incorrectly" << std::endl;
						else
							std::cerr << "    Multiplication dealiasing affected spectrum without need" << std::endl;
						std::cout << "    h1*h2 nonzero spectrum entries" << std::endl;
						(h1 * h2).print_spectralNonZero();
						FatalError("EXIT");
					}

					if (err_mult_dealias2 > eps)
					{
						std::cerr << " WARNING: error for multiplication function 'mult' too high !" << std::endl;
						FatalError("EXIT");
					}

#else
					if (err_mult_dealias > eps || err_mult_dealias2 > eps)
					std::cerr << " Turn on de-aliasing on compile time to analyse de-aliasing errors" << std::endl;

					if (err_mult > eps)
					{
						std::cerr << " Error threshold for multiplication operator too high !" << std::endl;
						FatalError("EXIT");
					}
#endif

					std::cout << " " << std::endl;

				}
			}

			std::cout << "TEST B: DONE" << std::endl;
#endif
		}

	}


	std::cout << "SUCCESSFULLY FINISHED -- check warnings!" << std::endl;

	return 0;
}
