#ifndef WBC_TYPES_HPP
#define WBC_TYPES_HPP

#include <string>
#include <vector>

namespace wbc{

enum task_type{task_type_joint, task_type_cartesian};


/**
 * @brief Defines a sub task in the whole body control problem. Valid Configuration are e.g.
 *        - task_type = wbc::joint
 *          root_frame = "Robot_base"
 *          tip_frame = "Gripper"
 *
 *        - task_type = WBC_TASK_TYPE_JOINT
 *          joints_ = ["J_1", "J_2", "J_3"]
 */
class SubTaskConfig{
public:
    SubTaskConfig(){}
    SubTaskConfig(const task_type _type,
                  const unsigned int _priority,
                  const std::string& _name,
                  const std::vector<std::string>& _joints,
                  const std::string &_root = "",
                  const std::string &_tip = "") :
        type(_type),
        priority(_priority),
        name(_name),
        joints(_joints),
        root(_root),
        tip(_tip){}


    /** Whole body task type, can be joint space or Cartesian for now */
    task_type type;

    /** Priority of this subtask. 0-based. 0 ^= highest priority */
    unsigned int priority;

    /** Unique identifier of the task*/
    std::string name;

    /** Only Joint Space Tasks: In case the task is of type WBC_TASK_TYPE_JOINT,
     * the joints used for this task have to be specified here.
     */
    std::vector<std::string> joints;

    /**
     * Only Cartesian Tasks: Root frame associated with this task.
     * Has to be the name of a link available in robot`s KDL tree.
     * This parameter is neglected if the task is in joint space
     */
    std::string root;

    /** Only Cartesian Tasks: Tip frame associated with this task.
     *  Has to be the name of a link available in robot`s KDL tree or an empty string.
     *  If empty, the task is assumed to in joint space
     */
    std::string tip;
};
}
#endif
