#include "PDESWESphere_n.hpp"


#include <vector>
#include <sweet/core/sphere/SphereOperators.hpp>

PDESWESphere_n::PDESWESphere_n()	:
	shackPDESWESphere(nullptr),
	ops(nullptr),
	dt(-1)
{
	setEvalAvailable("tendencies");
}

PDESWESphere_n::~PDESWESphere_n()
{
}


bool PDESWESphere_n::shackRegistration(
		sweet::ShackDictionary *io_shackDict
)
{
	shackPDESWESphere = io_shackDict->getAutoRegistration<ShackPDESWESphere>();
	ERROR_CHECK_WITH_FORWARD_AND_COND_RETURN_BOOLEAN(*io_shackDict);

	return true;
}

const std::vector<std::string> PDESWESphere_n::getNodeNames()
{
	std::vector<std::string> retval;
	retval.push_back("n");
	return retval;

}

std::shared_ptr<sweet::DESolver_TimeTreeNode_Base> PDESWESphere_n::getNewInstance()
{
	return std::shared_ptr<sweet::DESolver_TimeTreeNode_Base>(new PDESWESphere_n);
}


bool PDESWESphere_n::setupConfigAndGetTimeStepperEval(
	const sweet::DESolver_Config_Base &i_deTermConfig,
	const std::string &i_timeStepperEvalName,
	DESolver_TimeTreeNode_Base::EvalFun &o_timeStepper
)
{
	const PDESWESphere_DESolver_Config& myConfig = cast(i_deTermConfig);

	ops = myConfig.ops;

	if (shackPDESWESphere->sphere_use_fsphere)
		fg = ops->getFG_fSphere(shackPDESWESphere->sphere_fsphere_f0);
	else
		fg = ops->getFG_rotatingSphere(shackPDESWESphere->sphere_rotating_coriolis_omega);

	ug.setup(ops->sphereDataConfig);
	vg.setup(ops->sphereDataConfig);


	// default setup
	DESolver_TimeTreeNode_Base::_helperSetupConfigAndGetTimeStepperEval(
			i_timeStepperEvalName,
			o_timeStepper
		);
	ERROR_CHECK_WITH_FORWARD_AND_COND_RETURN_BOOLEAN(*this);

	return true;
	//return error.set("Time evaluation '"+i_timeStepperEvalName+"' not supported");
}


void PDESWESphere_n::setTimeStepSize(double i_dt)
{
	dt = i_dt;
}

void PDESWESphere_n::clear()
{
}

/*
 * Return the time tendencies of the PDE term
 */
void PDESWESphere_n::_eval_tendencies(
		const sweet::DESolver_DataContainer_Base &i_U_,
		sweet::DESolver_DataContainer_Base &o_U_,
		double i_time_stamp
)
{
	const PDESWESphere_DataContainer &i_U = cast(i_U_);
	PDESWESphere_DataContainer &o_U = cast(o_U_);

	assert(ops != nullptr);
	assert(shackPDESWESphere != nullptr);


	/*
	 * NON-LINEAR
	 *
	 * Follows Hack & Jakob formulation
	 */

	sweet::SphereData_Physical ug(i_U.phi_pert.sphereDataConfig);
	sweet::SphereData_Physical vg(i_U.phi_pert.sphereDataConfig);

	sweet::SphereData_Physical vrtg = i_U.vrt.toPhys();
	sweet::SphereData_Physical divg = i_U.div.toPhys();
	ops->vrtdiv_to_uv(i_U.vrt, i_U.div, ug, vg);

	sweet::SphereData_Physical phig = i_U.phi_pert.toPhys();

	sweet::SphereData_Physical tmpg1 = ug*(vrtg/*+fg*/);
	sweet::SphereData_Physical tmpg2 = vg*(vrtg/*+fg*/);

	ops->uv_to_vrtdiv(tmpg1, tmpg2, o_U.div, o_U.vrt);

	o_U.vrt *= -1.0;

	tmpg1 = ug*phig;
	tmpg2 = vg*phig;

	sweet::SphereData_Spectral tmpspec(i_U.phi_pert.sphereDataConfig);
	ops->uv_to_vrtdiv(tmpg1,tmpg2, tmpspec, o_U.phi_pert);

	o_U.phi_pert *= -1.0;

	sweet::SphereData_Physical tmpg = 0.5*(ug*ug+vg*vg);

	tmpspec = /*phig+*/tmpg;

	o_U.div += -ops->laplace(tmpspec);
}
