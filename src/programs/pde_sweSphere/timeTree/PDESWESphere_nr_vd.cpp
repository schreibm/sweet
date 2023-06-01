#include "PDESWESphere_nr_vd.hpp"


#include <vector>
#include <sweet/core/sphere/SphereOperators.hpp>

PDESWESphere_nr_vd::PDESWESphere_nr_vd()	:
	shackPDESWESphere(nullptr),
	ops(nullptr),
	dt(-1)
{
	setEvalAvailable("tendencies");
}

PDESWESphere_nr_vd::~PDESWESphere_nr_vd()
{
}


bool PDESWESphere_nr_vd::shackRegistration(
		sweet::ShackDictionary *io_shackDict
)
{
	shackPDESWESphere = io_shackDict->getAutoRegistration<ShackPDESWESphere>();
	ERROR_CHECK_WITH_FORWARD_AND_COND_RETURN_BOOLEAN(*io_shackDict);

	return true;
}

const std::vector<std::string> PDESWESphere_nr_vd::getNodeNames()
{
	std::vector<std::string> retval;
	retval.push_back("nr_vd");
	return retval;

}

std::shared_ptr<sweet::DESolver_TimeTreeNode_Base> PDESWESphere_nr_vd::getNewInstance()
{
	return std::shared_ptr<sweet::DESolver_TimeTreeNode_Base>(new PDESWESphere_nr_vd);
}


bool PDESWESphere_nr_vd::setupConfigAndGetTimeStepperEval(
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

void PDESWESphere_nr_vd::setTimeStepSize(double i_dt)
{
	dt = i_dt;
}

void PDESWESphere_nr_vd::clear()
{
}

/*
 * Return the time tendencies of the PDE term
 */
void PDESWESphere_nr_vd::_eval_tendencies(
		const sweet::DESolver_DataContainer_Base &i_U_,
		sweet::DESolver_DataContainer_Base &o_U_,
		double i_time_stamp
)
{
	const PDESWESphere_DataContainer &i_U = cast(i_U_);
	PDESWESphere_DataContainer &o_U = cast(o_U_);

	assert(ops != nullptr);
	assert(shackPDESWESphere != nullptr);


	sweet::SphereData_Physical U_u_phys, U_v_phys;
	ops->vrtdiv_to_uv(i_U.vrt, i_U.div, U_u_phys, U_v_phys);

	// dt calculation starts here

	sweet::SphereData_Physical U_div_phys = i_U.div.toPhys();

	o_U.phi_pert = sweet::SphereData_Spectral(i_U.phi_pert.toPhys()*i_U.div.toPhys());

	if (0)
	{
		o_U.vrt = o_U.vrt.toPhys()*U_div_phys;
	}
	else
	{

		/*
		 * N from UV formulation
		 */
//		double gh0 = shackPDESWESphere->gravitation * shackPDESWESphere->h0;


//		const sweet::SphereData_Spectral &U_phi = i_U.phi_pert;
		//const sweet::SphereData_Spectral &U_vrt = i_U.vrt;
		const sweet::SphereData_Spectral &U_div = i_U.div;


		sweet::SphereData_Physical U_u_phys, U_v_phys;
		ops->vrtdiv_to_uv(i_U.vrt, i_U.div, U_u_phys, U_v_phys);

		sweet::SphereData_Physical U_div_phys = U_div.toPhys();

		/*
		 * Velocity
		 */
		sweet::SphereData_Physical vrtg = i_U.vrt.toPhys();

		sweet::SphereData_Physical u_nl = U_u_phys*vrtg;
		sweet::SphereData_Physical v_nl = U_v_phys*vrtg;

		sweet::SphereData_Spectral vrt, div;
		ops->uv_to_vrtdiv(u_nl, v_nl, vrt, div);
		//o_vrt_t -= div;


		/*
		 * NA part to be subtracted
		 */
		sweet::SphereData_Spectral phi_tmp(i_U.phi_pert.sphereDataConfig);
		sweet::SphereData_Spectral vrt_tmp(i_U.vrt.sphereDataConfig);
		sweet::SphereData_Spectral div_tmp(i_U.div.sphereDataConfig);

		phi_tmp.spectral_set_zero();
		vrt_tmp.spectral_set_zero();
		div_tmp.spectral_set_zero();

#if 0
		euler_timestep_update_na(
				i_U.phi_pert, i_U.vrt, i_U.div,
				phi_tmp, vrt_tmp, div_tmp,
				i_time_stamp
			);
#else
		phi_tmp = ops->V_dot_grad_scalar(U_u_phys, U_v_phys, U_div_phys, i_U.phi_pert.toPhys());
		vrt_tmp = ops->V_dot_grad_scalar(U_u_phys, U_v_phys, U_div_phys, i_U.vrt.toPhys());
		div_tmp = ops->V_dot_grad_scalar(U_u_phys, U_v_phys, U_div_phys, i_U.div.toPhys());
#endif

		o_U.vrt = -div - vrt_tmp;
	}

	const sweet::SphereData_Physical U_vrt_phys = i_U.vrt.toPhys();
	o_U.div = ops->uv_to_vort(U_vrt_phys*U_u_phys, U_vrt_phys*U_v_phys);
	o_U.div += ops->uv_to_div(U_div_phys*U_u_phys, U_div_phys*U_v_phys);
	o_U.div -= 0.5*ops->laplace(U_u_phys*U_u_phys + U_v_phys*U_v_phys);
	o_U.div -= U_div_phys*U_div_phys;
}
