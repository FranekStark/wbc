#ifndef VELOCITYSCENEQUADRATICCOST_HPP
#define VELOCITYSCENEQUADRATICCOST_HPP

#include "../scenes/VelocityScene.hpp"

namespace wbc{

/**
 * Same as WbcVelocityScene, only that tasks A*x=y are implemented as part of the cost function x^T*H*x + x^T*g
 * as follows: H = A^T*A and g = -(A^T*y)^T. Variable damping can be applied optionally to guide the robot safely
 * through kinematic singularities.
 */
class VelocitySceneQuadraticCost : public VelocityScene{
protected:
    double min_eval_damping_thresh, damping_factor;
    base::VectorXd s_vals, tmp;
    base::MatrixXd sing_vect_r, U;

public:
    /**
     * @brief WbcVelocityScene
     * @param robot_model Pointer to the robot model
     * @param model_tasks_as_constraints Model tasks as constraints (true) or as part of the cost function
     */
    VelocitySceneQuadraticCost(RobotModelPtr robot_model, QPSolverPtr solver);
    virtual ~VelocitySceneQuadraticCost();

    /**
     * @brief Update the wbc scene
     */
    virtual const HierarchicalQP& update();

    void setDampingThreshold(double thresh){min_eval_damping_thresh = thresh;}
    double getCurrentDampingFactor(){return damping_factor;}
};

} // namespace wbc

#endif
