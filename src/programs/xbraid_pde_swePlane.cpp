/*
 * Author: Joao STEINSTRAESSER <joao.steinstraesser@usp.br>
 *
 * MULE_COMPILE_FILES_AND_DIRS: src/programs/pde_swePlane/
 * MULE_COMPILE_FILES_AND_DIRS: src/programs/pde_swePlane/time/
 * MULE_COMPILE_FILES_AND_DIRS: src/programs/pde_swePlane/benchmarks/
 *
 */

#include "pde_swePlane/ProgramXBraidPDESWEPlane.hpp"

int main(int i_argc, char *i_argv[])
{

#if SWEET_MPI
	int mpi_rank;
	MPI_Init(&i_argc, &i_argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
#endif

	ProgramXBraidPDESWEPlane simulation(
						i_argc, i_argv
#if SWEET_MPI
						,
						MPI_COMM_WORLD,
						mpi_rank
#endif
			);
	ERROR_CHECK_WITH_PRINT_AND_COND_RETURN_EXITCODE(simulation);

	simulation.setup();
	ERROR_CHECK_WITH_PRINT_AND_COND_RETURN_EXITCODE(simulation);

	{
		simulation.shackTimestepControl->validateMaxSimulationTimeOrTimestepNr();
		ERROR_CHECK_WITH_PRINT_AND_COND_RETURN_EXITCODE(*(simulation.shackTimestepControl));

		simulation.runXBraid();

	}

	///simulation.printSimulationErrors();
	ERROR_CHECK_WITH_PRINT_AND_COND_RETURN_EXITCODE(simulation);

	std::cout << "FIN" << std::endl;

#if SWEET_MPI
	MPI_Finalize();
#endif

	return 0;
}
