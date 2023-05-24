#ifndef SRC_PROGRAMS_SIMDATA_TIMESTEPPERPDETERM_N_HPP_
#define SRC_PROGRAMS_SIMDATA_TIMESTEPPERPDETERM_N_HPP_


/*
 * Generic includes
 */
#include <sweet/core/ErrorBase.hpp>
#include "../ShackPDESWESphere.hpp"

/*
 * Time tree node related includes
 */
#include <sweet/timeTree/DESolver_TimeTreeNode_Base.hpp>
#include <sweet/timeTree/DESolver_TimeTreeNode_CastHelper.hpp>
#include "PDESWESphere_DataContainer.hpp"
#include "PDESWESphere_DESolver_Config.hpp"


class PDESWESphere_n	:
	public sweet::DESolver_TimeTreeNode_Base,
	public sweet::DESolver_TimeTreeNode_CastHelper<
			PDESWESphere_DataContainer,
			PDESWESphere_DESolver_Config
		>
{
private:
	ShackPDESWESphere *shackPDESWESphere;
	const sweet::SphereOperators *ops;

	double dt;

	/*
	 * Coriolis effect
	 */
	sweet::SphereData_Physical fg;

	/*
	 * Temporary variables
	 */
	sweet::SphereData_Physical ug;
	sweet::SphereData_Physical vg;

public:
	PDESWESphere_n();
	~PDESWESphere_n();

public:
	bool shackRegistration(
			sweet::ShackDictionary *io_shackDict
	) override;

	virtual
	const std::vector<std::string> getNodeNames();

	std::shared_ptr<sweet::DESolver_TimeTreeNode_Base> getNewInstance() override;

	virtual
	bool setupConfig(
		const sweet::DESolver_Config_Base &i_deTermConfig
	) override;

	void setTimeStepSize(double i_dt) override;

	void clear() override;

	/*
	 * Return the time tendencies of the PDE term
	 */
	void eval_tendencies(
			const sweet::DESolver_DataContainer_Base &i_u,
			sweet::DESolver_DataContainer_Base &o_u,
			double i_time_stamp
	)	override;
};


#endif
