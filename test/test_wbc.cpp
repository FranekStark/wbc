#include <kdl/chainfksolverpos_recursive.hpp>
#include <kdl/frames_io.hpp>
#include <kdl/chainfksolvervel_recursive.hpp>
#include <boost/test/unit_test.hpp>
#include <stdlib.h>
#include <kdl_parser/kdl_parser.hpp>
#include "../src/Constraint.hpp"
#include "../src/WbcVelocity.hpp"
#include "../src/HierarchicalWDLSSolver.hpp"
#include "../src/GeneralizedInverse.hpp"

using namespace std;
using namespace wbc;

/**
 * Test hierarchical solver with random input data
 */
/*
BOOST_AUTO_TEST_CASE(solver)
{
    srand (time(NULL));

    const uint NO_JOINTS = 2;
    const uint NO_CONSTRAINTS = 3;
    const double NORM_MAX = 5.75;

    HierarchicalWDLSSolver solver;
    std::vector<uint> ny_per_prio(1,NO_CONSTRAINTS);
    BOOST_CHECK_EQUAL(solver.configure(ny_per_prio, NO_JOINTS), true);
    solver.setNormMax(NORM_MAX);

    cout<<"............Testing Hierarchical Solver ............ "<<endl<<endl;

    SolverInput input;
    input.priorities.push_back(SolverInputPrio(NO_CONSTRAINTS,NO_JOINTS));

    for(uint i = 0; i < NO_CONSTRAINTS*NO_JOINTS; i++ )
        input.priorities[0].A.data()[i] = (rand()%1000)/1000.0;

    for(uint i = 0; i < NO_CONSTRAINTS; i++ )
        input.priorities[0].y_ref.data()[i] = (rand()%1000)/1000.0;

    input.priorities[0].Wy(0) = 0.1;

    cout<<"............Testing Hierarchical Solver "<<endl<<endl;
    cout<<"Number of priorities: "<<ny_per_prio.size()<<endl;
    cout<<"Constraints per priority: "; for(uint i = 0; i < ny_per_prio.size(); i++) cout<<ny_per_prio[i]<<" "; cout<<endl;
    cout<<"No of joints: "<<NO_JOINTS<<endl;
    cout<<"\nSolver Input: "<<endl;
    for(uint i = 0; i < ny_per_prio.size(); i++){
        cout<<"Priority: "<<i<<endl;
        cout<<"A: "<<endl; cout<<input.priorities[i].A<<endl;
        cout<<"y: "<<endl; cout<<input.priorities[i].y_ref<<endl;
        cout<<endl;
    }

    Eigen::VectorXd solver_output;
    try{
        solver.solve(input,  solver_output);
    }
    catch(std::exception e){
        BOOST_ERROR("Solver.solve threw an exception");
    }

    cout<<"Solver Output: "<<solver_output<<endl;
    cout<<"\nTest: "<<endl;
    for(uint i = 0; i < ny_per_prio.size(); i++){
        cout<<"Priority: "<<i<<endl;
        Eigen::VectorXd test = input.priorities[i].A*solver_output;
        cout<<"A*q: "<<test<<endl; cout<<endl;
        for(uint j = 0; j < NO_CONSTRAINTS; j++)
            BOOST_CHECK_EQUAL(fabs(test(j) - input.priorities[i].y_ref(j)) < 1e-9, true);
    }

    cout<<"\n............................."<<endl;
}*/

BOOST_AUTO_TEST_CASE(pseudo_inverse)
{
    srand (time(NULL));
    const uint N_ROWS = 3;
    const uint N_COLS = 5;

    GeneralizedInverse inv(N_ROWS, N_COLS);

    Eigen::MatrixXd in(N_ROWS, N_COLS);
    Eigen::MatrixXd out(N_COLS, N_ROWS);

    for(uint i = 0; i < N_ROWS*N_COLS; i++ )
        in.data()[i] = (rand()%1000)/1000.0;

    inv.computeInverse(in, out);

    Eigen::MatrixXd res(N_ROWS,N_ROWS);
    res = in * out;

    std::cout << "--------- Input Mat --------" << std::endl << std::endl;
    std::cout << in << std::endl << std::endl;

    std::cout << "Weighting time: " << inv.time_weighting_ << " seconds " << std::endl << std::endl;
    std::cout << "SVD time: " << inv.time_svd_ << " seconds " << std::endl << std::endl;
    std::cout << "Multiplication time: " << inv.time_multiplying_ << " seconds " << std::endl << std::endl;
    std::cout << "Total Computation time: " << inv.time_total_ << " seconds " << std::endl << std::endl;

    std::cout << "--------- Output Mat --------" << std::endl << std::endl;
    std::cout<< out << std::endl << std::endl;

    std::cout << " ----- Input * Output Mat: ----- " << std::endl << std::endl;
    std::cout<< res << std::endl;

    for(uint i = 0; i < N_ROWS; i++)
    {
        for(uint j = 0; j < N_ROWS; j++)
        {
            if(i == j)
                BOOST_CHECK_EQUAL(fabs(res(i,j) - 1 )  < 1e-5, true);
            else
                BOOST_CHECK_EQUAL(fabs(res(i,j))  < 1e-5, true);
        }
    }
}

BOOST_AUTO_TEST_CASE(damped_pseudo_inverse)
{
    srand (time(NULL));
    const uint N_ROWS = 3;
    const uint N_COLS = 5;

    GeneralizedInverse inv(N_ROWS, N_COLS);
    inv.setConstantDamping(0.01);

    Eigen::MatrixXd in(N_ROWS, N_COLS);
    Eigen::MatrixXd out(N_COLS, N_ROWS);

    for(uint i = 0; i < N_ROWS*N_COLS; i++ )
        in.data()[i] = (rand()%1000)/1000.0;

    inv.computeInverse(in, out);

    Eigen::MatrixXd res(N_ROWS,N_ROWS);
    res = in * out;

    std::cout << "--------- Input Mat --------" << std::endl << std::endl;
    std::cout << in << std::endl << std::endl;

    std::cout << "Weighting time: " << inv.time_weighting_ << " seconds " << std::endl << std::endl;
    std::cout << "SVD time: " << inv.time_svd_ << " seconds " << std::endl << std::endl;
    std::cout << "Multiplication time: " << inv.time_multiplying_ << " seconds " << std::endl << std::endl;
    std::cout << "Total Computation time: " << inv.time_total_ << " seconds " << std::endl << std::endl;
    std::cout << " Current Damping: " << inv.damping_ << std::endl << std::endl;

    std::cout << "--------- Output Mat --------" << std::endl << std::endl;
    std::cout<< out << std::endl << std::endl;

    std::cout << " ----- Input * Output Mat: ----- " << std::endl << std::endl;
    std::cout<< res << std::endl;

    for(uint i = 0; i < N_ROWS; i++)
    {
        for(uint j = 0; j < N_ROWS; j++)
        {
            if(i == j)
                BOOST_CHECK_EQUAL(fabs(res(i,j) - 1 )  < 0.01, true);
            else
                BOOST_CHECK_EQUAL(fabs(res(i,j))  < 0.01, true);
        }
    }
}

BOOST_AUTO_TEST_CASE(auto_damped_pseudo_inverse)
{
    srand (time(NULL));
    const uint N_ROWS = 3;
    const uint N_COLS = 5;
    const double NORM_MAX = 2.0;

    GeneralizedInverse inv(N_ROWS, N_COLS);
    inv.setNormMaxDamping(NORM_MAX);

    Eigen::MatrixXd in(N_ROWS, N_COLS);
    Eigen::MatrixXd out(N_COLS, N_ROWS);

    for(uint i = 0; i < N_ROWS*N_COLS; i++ )
        in.data()[i] = (rand()%1000)/1000.0;

    inv.computeInverse(in, out);

    Eigen::MatrixXd res(N_ROWS,N_ROWS);
    res = in * out;

    std::cout << "--------- Input Mat --------" << std::endl << std::endl;
    std::cout << in << std::endl << std::endl;

    std::cout << "Norm Max: " << NORM_MAX << std::endl << std::endl;
    std::cout << "1 /Norm Max: " << 1/inv.norm_max_ << std::endl << std::endl;
    std::cout << "Current Damping: " << inv.damping_ << std::endl << std::endl;

    std::cout << "--------- Singular Values --------" << std::endl << std::endl;
    std::cout << inv.singular_vals_ << std::endl << std::endl;

    std::cout << "Weighting time: " << inv.time_weighting_ << " seconds " << std::endl << std::endl;
    std::cout << "SVD time: " << inv.time_svd_ << " seconds " << std::endl << std::endl;
    std::cout << "Multiplication time: " << inv.time_multiplying_ << " seconds " << std::endl << std::endl;
    std::cout << "Total Computation time: " << inv.time_total_ << " seconds " << std::endl << std::endl;

    std::cout << "--------- Output Mat --------" << std::endl << std::endl;
    std::cout<< out << std::endl << std::endl;

    std::cout << " ----- Input * Output Mat: ----- " << std::endl << std::endl;
    std::cout<< res << std::endl;

    for(uint i = 0; i < N_ROWS; i++)
    {
        for(uint j = 0; j < N_ROWS; j++)
        {
            if(i == j)
                BOOST_CHECK_EQUAL(fabs(res(i,j) - 1 )  < 0.01, true);
            else
                BOOST_CHECK_EQUAL(fabs(res(i,j))  < 0.01, true);
        }
    }
}

BOOST_AUTO_TEST_CASE(weighted_pseudo_inverse)
{
    srand (time(NULL));
    const uint N_ROWS = 3;
    const uint N_COLS = 5;
    const uint COL_NUMBER_WITH_ZERO_WEIGHT = 3;
    const uint ROW_NUMBER_WITH_ZERO_WEIGHT = 1;

    GeneralizedInverse inv(N_ROWS, N_COLS);

    Eigen::MatrixXd in(N_ROWS, N_COLS);
    Eigen::MatrixXd out(N_COLS, N_ROWS);

    Eigen::VectorXd col_weights, row_weights;
    col_weights.setOnes(N_COLS);
    row_weights.setOnes(N_ROWS);

    col_weights(COL_NUMBER_WITH_ZERO_WEIGHT) = 0;
    row_weights(ROW_NUMBER_WITH_ZERO_WEIGHT) = 0;

    inv.setColWeights(col_weights);
    inv.setRowWeights(row_weights);

    for(uint i = 0; i < N_ROWS*N_COLS; i++ )
        in.data()[i] = (rand()%1000)/1000.0;

    inv.computeInverse(in, out);

    Eigen::MatrixXd res(N_ROWS,N_ROWS);
    res = in * out;

    std::cout << "--------- Input Mat --------" << std::endl << std::endl;
    std::cout << in << std::endl << std::endl;

    std::cout << "---------- Column weights -------- " << std::endl << std::endl;
    std::cout << inv.col_weights_ << std::endl << std::endl;

    std::cout << "---------- Row weights -------- " << std::endl << std::endl;
    std::cout << inv.row_weights_ << std::endl << std::endl;

    std::cout << "--------- Weighted input Mat --------" << std::endl << std::endl;
    std::cout << inv.weighted_mat_ << std::endl << std::endl;

    std::cout << "--------- Output Mat --------" << std::endl << std::endl;
    std::cout<< out << std::endl << std::endl;

    std::cout << "Weighting time: " << inv.time_weighting_ << " seconds " << std::endl << std::endl;
    std::cout << "SVD time: " << inv.time_svd_ << " seconds " << std::endl << std::endl;
    std::cout << "Multiplication time: " << inv.time_multiplying_ << " seconds " << std::endl << std::endl;
    std::cout << "Total Computation time: " << inv.time_total_ << " seconds " << std::endl << std::endl;

    std::cout << " ----- Input * Output Mat: ----- " << std::endl << std::endl;
    std::cout<< res << std::endl;

    for(uint i = 0; i < N_ROWS; i++)
    {
        for(uint j = 0; j < N_ROWS; j++)
        {
            if( i == ROW_NUMBER_WITH_ZERO_WEIGHT)
                continue;

            if(i == j)
                BOOST_CHECK_EQUAL(fabs(res(i,j) - 1 )  < 1e-5, true);
            else
                BOOST_CHECK_EQUAL(fabs(res(i,j))  < 1e-5, true);
        }
    }
}

/*
BOOST_AUTO_TEST_CASE(wbc_cart_aila)
{
    srand (time(NULL));

    int argc = boost::unit_test::framework::master_test_suite().argc;
    char **argv = boost::unit_test::framework::master_test_suite().argv;

    argc-=2;
    argv+=2;

    std::string urdf_file = argv[0];

    KDL::Tree tree, reduced_tree;
    BOOST_CHECK_EQUAL(kdl_parser::treeFromFile(urdf_file, tree), true);
    KDL::Chain right_hand_chain;
    tree.getChain("Rover_base", "Hand_r", right_hand_chain);
    reduced_tree.addSegment(KDL::Segment("Rover_base", KDL::Joint("Rover_base",KDL::Joint::None),KDL::Frame::Identity()), "root");
    reduced_tree.addChain(right_hand_chain, "Rover_base");

    WbcVelocity wbc;
    std::vector<ConstraintConfig> config;
    ConstraintConfig conf(cart, 0, "Cart_r", "Chest", "Hand_r");
    config.push_back(conf);

    std::vector<std::string> joint_names;
    for(uint i = 0; i < right_hand_chain.getNrOfSegments(); i++)
    {
        KDL::Segment seg = right_hand_chain.getSegment(i);
        if(seg.getJoint().getType() != KDL::Joint::None)
            joint_names.push_back(seg.getJoint().getName());
    }

    BOOST_CHECK_EQUAL(wbc.configure(reduced_tree, config, joint_names, true, 3), true);

    Constraint* sub_task = wbc.constraint(conf.name);
    sub_task->y_des = Eigen::VectorXd(6);
    for(uint i = 0; i < 6; i++)
        sub_task->y_des(i) = (rand()%1000)/1000.0;;

    base::samples::Joints status;
    status.resize(right_hand_chain.getNrOfJoints());
    status.names = joint_names;

    for(uint i = 0; i < status.size(); i++)
        status[i].position = 1.;

    Eigen::VectorXd x(status.size());
    wbc.solver()->setNormMax(20.0);
    wbc.solve(status, x);

    Eigen::VectorXd y_act(6);
    y_act = sub_task->A * x;

    for(uint i = 0; i < y_act.rows(); i++)
        BOOST_CHECK_EQUAL(fabs(y_act(i) - sub_task->y_des(i)) < 1e-5, true);

    cout<<"..........................................................."<<endl;
    cout<<"Desired y: "<<endl;
    cout<<sub_task->y_des.transpose()<<endl<<endl;
    cout<<"Joint names: "<<endl;
    for(uint i = 0; i <status.names.size(); i++) cout<<status.names[i]<<" "; cout<<endl<<endl;
    cout<<"Ctrl solution: "<<endl;
    cout<<x.transpose()<<endl<<endl;
    cout<<"Actual y: "<<endl;
    cout<<y_act.transpose()<<endl<<endl;
    cout<<"..........................................................."<<endl;

}

BOOST_AUTO_TEST_CASE(wbc_joint)
{
    int argc = boost::unit_test::framework::master_test_suite().argc;
    char **argv = boost::unit_test::framework::master_test_suite().argv;

    argc-=2;
    argv+=2;

    std::string urdf_file = argv[0];

    KDL::Tree tree, reduced_tree;
    BOOST_CHECK_EQUAL(kdl_parser::treeFromFile(urdf_file, tree), true);
    KDL::Chain right_hand_chain;
    tree.getChain("Rover_base", "Hand_r", right_hand_chain);
    reduced_tree.addSegment(KDL::Segment("Rover_base", KDL::Joint("Rover_base",KDL::Joint::None),KDL::Frame::Identity()), "root");
    reduced_tree.addChain(right_hand_chain, "Rover_base");

    WbcVelocity wbc;
    std::vector<ConstraintConfig> config;
    std::vector<std::string> joints;
    joints.push_back("J_Foot");
    joints.push_back("J_Knees");
    joints.push_back("J_Hip");
    joints.push_back("J_Waist");
    joints.push_back("J_Shoulder1_r");
    joints.push_back("J_Shoulder2_r");
    joints.push_back("J_UpperArm_r");
    joints.push_back("J_Elbow_r");
    joints.push_back("J_Forearm_r");
    joints.push_back("J_Wrist1_r");
    joints.push_back("J_Wrist2_r");
    ConstraintConfig conf(jnt, 0, "Upper_limits", "", "",  joints);
    config.push_back(conf);
    conf.name = "Lower_limits";
    config.push_back(conf);

    BOOST_CHECK_EQUAL(wbc.configure(reduced_tree, config, joints, true, 3), true);

    Constraint* sub_task = wbc.constraint("Upper_limits");
    sub_task->y_des(0) = 0.2;
    sub_task->y_des(1) = -0.3;

    sub_task = wbc.constraint("Lower_limits");

    for(uint i = 0; i < sub_task->config.joint_names.size(); i++)
        sub_task->weights(i,i) = 0;


    base::samples::Joints status;
    status.resize(right_hand_chain.getNrOfJoints());
    for(uint i = 0; i < status.size(); i++)
        status[i].position = 0.0;
    status.names = joints;

    Eigen::VectorXd x(status.size());
    wbc.solver()->setNormMax(10000.0);
    wbc.solve(status, x);

    for(uint task_no = 0; task_no < config.size(); task_no++){

        sub_task = wbc.constraint(config[task_no].name);
        Eigen::VectorXd y_act(11);
        y_act = sub_task->A * x;

        for(uint i = 0; i < y_act.rows(); i++)
            BOOST_CHECK_EQUAL(fabs(y_act(i) - sub_task->y_des(i)) < 1e-5, true);

        cout<<"..........................................................."<<endl;
        cout<<"Task: "<<config[task_no].name<<endl;
        cout<<"Desired y: "<<endl;
        cout<<sub_task->y_des.transpose()<<endl<<endl;
        cout<<"Joint names: "<<endl;
        for(uint i = 0; i < status.names.size(); i++) cout<<status.names[i]<<" "; cout<<endl<<endl;
        cout<<"Ctrl solution: "<<endl;
        cout<<x.transpose()<<endl<<endl;
        cout<<"Actual y: "<<endl;
        cout<<y_act.transpose()<<endl<<endl;
        cout<<"..........................................................."<<endl;
    }


}



BOOST_AUTO_TEST_CASE(test_wbc_invalid_sub_task)
{
    int argc = boost::unit_test::framework::master_test_suite().argc;
    char **argv = boost::unit_test::framework::master_test_suite().argv;

    argc-=2;
    argv+=2;

    std::string urdf_file = argv[0];

    KDL::Tree tree;
    BOOST_CHECK_EQUAL(kdl_parser::treeFromFile(urdf_file, tree), true);

    //TODO
}
*/
