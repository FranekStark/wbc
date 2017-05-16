#ifndef ROBOTMODELCONFIG_HPP
#define ROBOTMODELCONFIG_HPP

#include <base/samples/RigidBodyState.hpp>

namespace wbc{

class RobotModelConfig{
public:
    RobotModelConfig(){
        initial_pose.setTransform(Eigen::Affine3d::Identity());
    }
    RobotModelConfig(const std::string& _file,
                     const std::string& _hook = "",
                     const base::samples::RigidBodyState& _initial_pose = base::samples::RigidBodyState()) :
        file(_file),
        hook(_hook),
        initial_pose(_initial_pose){
    }

    std::string file;                            /** Path to robot model file*/
    std::string hook;                            /** Frame to which this robot model is attached in the overall model*/
    base::samples::RigidBodyState initial_pose;  /** Initial pose of this model relative to the hook frame*/
};
}
#endif // ROBOTMODELCONFIG_HPP