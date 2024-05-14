#include "RigidbodyDynamicsConstraint.hpp"

namespace wbc{

    void RigidbodyDynamicsConstraint::update(RobotModelPtr robot_model) {
        
        const ActiveContacts& contacts = robot_model->getActiveContacts();

        uint nj = robot_model->noOfJoints();
        uint na = robot_model->noOfActuatedJoints();
        uint nc = contacts.size();

        uint nv = reduced ? (nj + nc*3) : (nj + na + nc*3);

        if(reduced) // no torques in qp, consider only floating base dynamics
        {
            A_mtx.resize(6, nv);
            A_mtx.setZero();

            b_vec.resize(6);

            A_mtx.block(0,  0, 6, nj) =  robot_model->jointSpaceInertiaMatrix().topRows<6>();
            for(uint i = 0; i < contacts.size(); i++){
                if(contacts[i].active)
                    A_mtx.block(0, nj+i*3, 6, 3) = -robot_model->spaceJacobian(robot_model->worldFrame(), contacts.names[i]).topRows<3>().transpose().topRows<6>();
                b_vec = -robot_model->biasForces().topRows<6>();
            }
        }
        else
        {
            A_mtx.resize(nj, nv);
            A_mtx.setZero();
            
            b_vec.resize(nj);

            A_mtx.block(0,  0, nj, nj) =  robot_model->jointSpaceInertiaMatrix();
            A_mtx.block(0, nj, nj, na) = -robot_model->selectionMatrix().transpose();
            for(uint i = 0; i < contacts.size(); i++){
                if(contacts[i].active)
                    A_mtx.block(0, nj+na+i*3, nj, 3) = -robot_model->spaceJacobian(robot_model->worldFrame(), contacts.names[i]).topRows(3).transpose();
                b_vec = -robot_model->biasForces();
            }
        }

    }


} // namespace wbc
