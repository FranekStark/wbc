#include "AccelerationSceneTSID.hpp"
#include "core/RobotModel.hpp"
#include <base-logging/Logging.hpp>

#include "../constraints/RigidbodyDynamicsHardConstraint.hpp"
#include "../constraints/ContactsAccelerationHardConstraint.hpp"
#include "../constraints/JointLimitsAccelerationHardConstraint.hpp"

namespace wbc {

AccelerationSceneTSID::AccelerationSceneTSID(RobotModelPtr robot_model, QPSolverPtr solver, double dt) :
    WbcScene(robot_model,solver),
    hessian_regularizer(1e-8){
    
    // for now manually adding constraint to this scene (an option would be to take them during configuration)
    hard_constraints.resize(1);
    hard_constraints[0].push_back(std::make_shared<RigidbodyDynamicsHardConstraint>());
    hard_constraints[0].push_back(std::make_shared<ContactsAccelerationHardConstraint>());
    hard_constraints[0].push_back(std::make_shared<JointLimitsAccelerationHardConstraint>(dt));

}

TaskPtr AccelerationSceneTSID::createTask(const TaskConfig &config){

    if(config.type == cart)
        return std::make_shared<CartesianAccelerationTask>(config, robot_model->noOfJoints());
    else if(config.type == jnt)
        return std::make_shared<JointAccelerationTask>(config, robot_model->noOfJoints());
    else{
        LOG_ERROR("Task with name %s has an invalid task type: %i", config.name.c_str(), config.type);
        throw std::invalid_argument("Invalid task config");
    }
}

const HierarchicalQP& AccelerationSceneTSID::update(){

    if(!configured)
        throw std::runtime_error("AccelerationSceneTSID has not been configured!. PLease call configure() before calling update() for the first time!");

    if(tasks.size() != 1){
        LOG_ERROR("Number of priorities in AccelerationSceneTSID should be 1, but is %i", tasks.size());
        throw std::runtime_error("Invalid task configuration");
    }

    int prio = 0; // Only one priority is implemented here!
    uint nj = robot_model->noOfJoints();
    uint na = robot_model->noOfActuatedJoints();
    uint ncp = robot_model->getActiveContacts().size();

    // QP Size: (NJoints+NContacts*2*6 x NJoints+NActuatedJoints+NContacts*6)
    // Variable order: (acc,torque,f_ext)
    tasks_prio[prio].resize(nj+ncp*6,nj+na+ncp*6);
    tasks_prio[prio].H.setZero();
    tasks_prio[prio].g.setZero();

    ///////// Tasks

    // Walk through all tasks
    for(uint i = 0; i < tasks[prio].size(); i++){
        
        TaskPtr task = tasks[prio][i];

        task->checkTimeout();
        task->update(robot_model);

        // If the activation value is zero, also set reference to zero. Activation is usually used to switch between different
        // task phases and we don't want to store the "old" reference value, in case we switch on the task again
        if(task->activation == 0){
           task->y_ref.setZero();
           task->y_ref_root.setZero();
        }

        for(int i = 0; i < task->A.rows(); i++)
            task->Aw.row(i) = task->weights_root(i) * task->A.row(i) * task->activation * (!task->timeout);
        for(int i = 0; i < task->A.cols(); i++)
            task->Aw.col(i) = joint_weights[i] * task->Aw.col(i);

        tasks_prio[prio].H.block(0,0,nj,nj) += task->Aw.transpose()*task->Aw;
        tasks_prio[prio].g.segment(0,nj) -= task->Aw.transpose()*task->y_ref_root;
    }

    tasks_prio[prio].H.block(0,0,nj,nj).diagonal().array() += hessian_regularizer;


    ///////// Constraints

    size_t total_eqs = 0;
    for(auto hard_contraint : hard_constraints[prio]) {
        hard_contraint->update(robot_model);
        if(hard_contraint->type() != HardConstraint::bounds)
            total_eqs += hard_contraint->size();
    }

    // Note already performed at the beginning of the update (but does not consider additional constraints)
    tasks_prio[prio].A.resize(total_eqs, nj+na+ncp*6);
    tasks_prio[prio].lower_x.resize(nj+na+ncp*6);
    tasks_prio[prio].upper_y.resize(nj+na+ncp*6);
    tasks_prio[prio].lower_y.resize(total_eqs);
    tasks_prio[prio].upper_y.resize(total_eqs);

    tasks_prio[prio].A.setZero();
    tasks_prio[prio].lower_y.setConstant(-99999);
    tasks_prio[prio].upper_y.setConstant(+99999);
    tasks_prio[prio].lower_x.setConstant(-99999);
    tasks_prio[prio].upper_x.setConstant(+99999);

    total_eqs = 0;
    for(uint i = 0; i < hard_constraints[prio].size(); i++) {
        HardConstraint::Type type = hard_constraints[prio][i]->type();
        size_t c_size = hard_constraints[prio][i]->size();

        if(type == HardConstraint::bounds) {
            tasks_prio[prio].lower_x = hard_constraints[prio][i]->lb();
            tasks_prio[prio].upper_x = hard_constraints[prio][i]->ub();
        }
        else if (type == HardConstraint::equality) {
            tasks_prio[prio].A.middleRows(total_eqs, c_size) = hard_constraints[prio][i]->A();
            tasks_prio[prio].lower_y.segment(total_eqs, c_size) = hard_constraints[prio][i]->b();
            tasks_prio[prio].upper_y.segment(total_eqs, c_size) = hard_constraints[prio][i]->b();
        }
        else if (type == HardConstraint::inequality) {
            tasks_prio[prio].A.middleRows(total_eqs, c_size) = hard_constraints[prio][i]->A();
            tasks_prio[prio].lower_y.segment(total_eqs, c_size) = hard_constraints[prio][i]->lb();
            tasks_prio[prio].upper_y.segment(total_eqs, c_size) = hard_constraints[prio][i]->ub();
        }

        total_eqs += c_size;
    }
    
    tasks_prio.Wq = base::VectorXd::Map(joint_weights.elements.data(), robot_model->noOfJoints());
    tasks_prio.time = base::Time::now(); //  TODO: Use latest time stamp from all tasks!?
    return tasks_prio;
}

const base::commands::Joints& AccelerationSceneTSID::solve(const HierarchicalQP& hqp){

    // solve
    solver_output.resize(hqp[0].nq);
    solver->solve(hqp, solver_output);

    // Convert solver output: Acceleration and torque
    uint nj = robot_model->noOfJoints();
    uint na = robot_model->noOfActuatedJoints();
    solver_output_joints.resize(robot_model->noOfActuatedJoints());
    solver_output_joints.names = robot_model->actuatedJointNames();
    for(uint i = 0; i < robot_model->noOfActuatedJoints(); i++){
        const std::string& name = robot_model->actuatedJointNames()[i];
        uint idx = robot_model->jointIndex(name);
        solver_output_joints[name].acceleration = solver_output[idx];
        solver_output_joints[name].effort = solver_output[i+nj];
    }
    solver_output_joints.time = base::Time::now();

    /*std::cout<<"Acc:   "<<solver_output.segment(0,nj).transpose()<<std::endl;
    std::cout<<"Tau:   "<<solver_output.segment(nj,na).transpose()<<std::endl;
    std::cout<<"F_ext: "<<solver_output.segment(nj+na,12).transpose()<<std::endl<<std::endl;*/

    // Convert solver output: contact wrenches
    contact_wrenches.resize(robot_model->getActiveContacts().size());
    contact_wrenches.names = robot_model->getActiveContacts().names;
    for(uint i = 0; i < robot_model->getActiveContacts().size(); i++){
        contact_wrenches[i].force = solver_output.segment(nj+na+i*6,3);
        contact_wrenches[i].torque = solver_output.segment(nj+na+i*6+3,3);
    }

    contact_wrenches.time = base::Time::now();
    return solver_output_joints;
}

const TasksStatus& AccelerationSceneTSID::updateTasksStatus(){

    uint nj = robot_model->noOfJoints();
    solver_output_acc = solver_output.segment(0,nj);
    const base::samples::Joints& joint_state = robot_model->jointState(robot_model->jointNames());
    robot_acc.resize(nj);
    for(size_t i = 0; i < nj; i++)
        robot_acc(i) = joint_state[i].acceleration;

    for(uint prio = 0; prio < tasks.size(); prio++){
        for(uint i = 0; i < tasks[prio].size(); i++){
            TaskPtr task = tasks[prio][i];
            const std::string &name = task->config.name;

            tasks_status[name].time       = task->time;
            tasks_status[name].config     = task->config;
            tasks_status[name].activation = task->activation;
            tasks_status[name].timeout    = task->timeout;
            tasks_status[name].weights    = task->weights;
            tasks_status[name].y_ref      = task->y_ref_root;
            if(task->config.type == cart){
                const base::MatrixXd &jac = robot_model->spaceJacobian(task->config.root, task->config.tip);
                const base::Acceleration &bias_acc = robot_model->spatialAccelerationBias(task->config.root, task->config.tip);
                tasks_status[name].y_solution = jac * solver_output_acc + bias_acc;
                tasks_status[name].y          = jac * robot_acc + bias_acc;
            }
        }
    }

    return tasks_status;
}

}
