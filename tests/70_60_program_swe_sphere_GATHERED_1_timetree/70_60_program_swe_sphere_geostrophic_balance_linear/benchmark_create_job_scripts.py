#! /usr/bin/env python3

import sys
from itertools import product

from mule.JobMule import *
from mule.utils import exec_program
from mule.InfoError import *
from mule.parHelper import *

jg = JobGeneration()


"""
Compile parameters
"""
params_compile_sweet_mpi = ['enable', 'disable']
params_compile_threading = ['omp', 'off']
params_compile_thread_parallel_sum = ['enable', 'disable']

jg.compile.program = 'programs/pde_sweSphere'

jg.compile.plane_spectral_space = 'disable'
jg.compile.plane_spectral_dealiasing = 'disable'
jg.compile.sphere_spectral_space = 'enable'
jg.compile.sphere_spectral_dealiasing = 'enable'

#jg.compile.quadmath = 'enable'
jg.unique_id_filter = ['runtime.simparams', 'parallelization', 'runtime.benchmark', 'runtime.rexi_params']


"""
Runtime parameters
"""
params_runtime_timestep_sizes = [30]

jg.runtime.benchmark_name = 'geostrophic_balance_linear'

jg.runtime.space_res_spectral = 128
jg.runtime.space_res_physical = None

jg.runtime.compute_errors = 1

# run 10 time steps
jg.runtime.max_simulation_time = 10*30


# Use moderate CI-REXI values
# Set later on
#jg.runtime.rexi_method = 'ci'
jg.runtime.rexi_ci_n = 16
jg.runtime.rexi_ci_max_real = 1
jg.runtime.rexi_ci_max_imag = 1
jg.runtime.rexi_ci_mu = 0
jg.runtime.rexi_ci_primitive = 'circle'
jg.runtime.rexi_sphere_preallocation = 1

jg.runtime.instability_checks = 0
jg.runtime.verbosity = 10

# output results after end
jg.runtime.output_timestep_size = jg.runtime.max_simulation_time
jg.runtime.output_filename = "-"

# Set the number of SH transformation threads
jg.runtime.sh_setup_num_threads = None






o = f"order=2"
ts_methods = [
    [f"ERK(l,order=4)",        4,    4,    0],    # reference solution
    #['l_erk',        4,    4,    0],    # reference solution

    [f"ERK(l,{o})",        4,    4,    0],    # reference solution
    #['l_erk',        2,    2,    0],

    [f"SS(ERK(lg,{o}),ERK(lc,{o}),{o})",        4,    4,    0],    # reference solution
    #['lg_erk_lc_erk',        2,    2,    0],

    [f"IRK(l,{o})",        2,    2,    0],
    #['l_irk',        2,    2,    0],

    [f"SS(IRK(lg,{o}),ERK(lc,{o}),{o})",        2,    2,    0],
    #['lg_irk_lc_erk',        2,    2,    0],

    #[f"REXI(l,{o})",        2,    2,    0],
    #['l_exp',            2,    2,    0],

    [f"SS(REXI(lg),ERK(lc,{o}),{o})",        2,    2,    0],
]




#
# allow including this file
#
if __name__ == "__main__":

    #
    # Create job scripts
    #
    for tsm in ts_methods[1:]:

        jg.runtime.timestepping_method = tsm[0]
        jg.runtime.timestepping_order = tsm[1]
        jg.runtime.timestepping_order2 = tsm[2]

        if len(tsm) > 4:
            s = tsm[4]
            jg.runtime.load_from_dict(tsm[4])

        for jg.runtime.timestep_size in params_runtime_timestep_sizes:

            for (
                jg.compile.threading,
                jg.compile.rexi_thread_parallel_sum,
                jg.compile.sweet_mpi
            ) in product(
                params_compile_threading,
                params_compile_thread_parallel_sum,
                params_compile_sweet_mpi
            ):
                if 'exp_' in jg.runtime.timestepping_method or 'REXI(' in jg.runtime.timestepping_method:

                    if jg.compile.rexi_thread_parallel_sum == "enable":
                        if jg.compile.threading == "off":
                            continue

                    setupParallelization(jg)

                    jg.runtime.rexi_method = 'ci'
                    jg.gen_jobscript_directory()
                    jg.runtime.rexi_method = ''

                else:
                    jg.runtime.sh_setup_num_threads = None

                    if jg.compile.sweet_mpi == 'enable':
                        continue

                    if jg.compile.rexi_thread_parallel_sum == 'enable':
                        continue

                    setupParallelization(jg)

                    jg.gen_jobscript_directory()

