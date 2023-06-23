#ifndef VELOCITYSCENE_HPP
#define VELOCITYSCENE_HPP

#include "../../core/Scene.hpp"

namespace wbc{

/**
 * @brief Velocity-based implementation of the WBC Scene. It sets up and solves the following problem:
 *  \f[
 *        \begin{array}{ccc}
 *        minimize & \| \dot{\mathbf{q}} \|_2& \\
 *            \mathbf{\dot{q}} & & \\
 *           s.t. & \mathbf{J}_w\dot{\mathbf{q}}=\mathbf{v}_{d}
 *        \end{array}
 *  \f]
 * \f$\dot{\mathbf{q}}\f$ - Vector of robot joint velocities<br>
 * \f$\mathbf{v}_{d}\f$ - desired task space velocities of all tasks stacked in a vector<br>
 * \f$\mathbf{J}\f$ - task Jacobians of all tasks stacked in a single matrix<br>
 * \f$\mathbf{J}_w\f$ - Weighted task Jacobians<br>
 * \f$\mathbf{W}\f$ - Diagonal task weight matrix<br>
 *
 * The tasks are all modeled as linear equality tasks to the above optimization problem. The task hierarchies are kept, i.e., multiple priorities are possible, depending on the solver.
 */
class VelocityScene : public Scene{
protected:
    static SceneRegistry<VelocityScene> reg;

    base::VectorXd q, qd, qdd;

    /**
     * @brief Create a task and add it to the WBC scene
     */
    virtual TaskPtr createTask(const TaskConfig &config);

public:
    VelocityScene(RobotModelPtr robot_model, QPSolverPtr solver, const double dt);
    virtual ~VelocityScene(){
    }

    /**
     * @brief Update the wbc scene and setup the optimization problem
     */
    virtual const HierarchicalQP& update();

    /**
     * @brief Solve the given optimization problem
     * @return Solver output as joint velocity command
     */
    virtual const base::commands::Joints& solve(const HierarchicalQP& hqp);

    /**
     * @brief Compute y and y_solution for each task. y_solution denotes the task velocity that can be achieved
     *  with the solution generated by the solver and y denotes the actual joint velocity achieved by the robot.
     *  Both values can be used to evaluate the performance of WBC
     */
    virtual const TasksStatus &updateTasksStatus();
};

} // namespace wbc

#endif