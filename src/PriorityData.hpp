#ifndef PRIORITY_DATA_HPP
#define PRIORITY_DATA_HPP

#include <base/Eigen.hpp>
#include <base/float.h>
#include <base/time.h>

namespace wbc{
/**
 * @brief The PriorityData class contains debug information for a priority level
 */
class PriorityData{
public:
    PriorityData(){}

    PriorityData(const unsigned int ny, const unsigned int nx, const unsigned int prio){
        priority = prio;
        no_constraints = ny;
        no_joints = nx;

        y_des.resize(ny);
        y_des.setConstant(base::NaN<double>());
        y_solution.resize(ny);
        y_solution.setConstant(base::NaN<double>());
        singular_vals.resize(nx);
        singular_vals.setConstant(base::NaN<double>());

        error_ratio = base::NaN<double>();
        damping = base::NaN<double>();
        damping_error_ratio = base::NaN<double>();
    }
    base::Time time;
    uint priority;
    uint no_constraints;
    uint no_joints;
    base::VectorXd y_des;         /** Reference vector */
    base::VectorXd y_solution;    /** Solution vector computed by the solver for this priority. This might differ from the desired values y_des because of the introduced damping for
                                      inversion of the constraint matrix, because of degeneracy of the Nullspace projector or of the Jacobian on this priority itself*/
    base::VectorXd singular_vals; /** Singular values of the projected, weighted constraint matrix on this priority */

    double damping;          /** Damping used on that priority for inverse computation */

    double error_ratio;                    /** Ratio of vector magnitude between desired and actual solution ||y_des|| / ||y_solution|| */
    double projected_manipulability_ratio; /** Ratio of the manipulability of the projected constraint matrix, to the unprojected one */
    double damping_error_ratio;            /** Ratio of vector norm of the solution obtained at the current priority to the solution that would have been obtained using
                                              undamped inverse*/
    double projection_error_ratio;         /** */
};
}

#endif
