#include "WbcScene.hpp"

namespace wbc{

void WbcScene::clearConstraints(){

    for(uint i = 0; i < constraints.size(); i++ ){
        for(uint j = 0; j < constraints[i].size(); j++)
            delete constraints[i][j];
        constraints[i].clear();
    }
    constraints.clear();
}

void WbcScene::configure(const std::vector<ConstraintConfig> &config){

    clearConstraints();

    if(config.empty())
        throw std::invalid_argument("WbcScene: Constraint configuration is empty");

    std::vector< std::vector<ConstraintConfig> > sorted_config;
    sortConstraintConfig(config, sorted_config);

    //// Create constraints. Store the number of constraint variables per priority
    ///
    constraints.resize(sorted_config.size());
    for(size_t i = 0; i < sorted_config.size(); i++){

        constraints[i].resize(sorted_config[i].size());
        for(size_t j = 0; j < sorted_config[i].size(); j++)
            constraints[i][j] = createConstraint(sorted_config[i][j]);
    }
    n_constraint_variables_per_prio = getNConstraintVariablesPerPrio(config);
}

Constraint* WbcScene::getConstraint(const std::string& name){

    for(size_t i = 0; i < constraints.size(); i++){
        for(size_t j = 0; j < constraints[i].size(); j++){
            if(constraints[i][j]->config.name == name)
                return constraints[i][j];
        }
    }
    throw std::invalid_argument("Invalid constraint name: " + name);
}

std::vector<ConstraintsPerPrio> WbcScene::getConstraints(){

    constraint_vector.resize(constraints.size());

    for(size_t i = 0; i < constraints.size(); i++){

        constraint_vector[i].resize(constraints[i].size());
        for(size_t j = 0; j < constraints[i].size(); j++)
            constraint_vector[i][j] = *constraints[i][j];
    }
    return constraint_vector;
}

bool WbcScene::hasConstraint(const std::string &name){

    for(size_t i = 0; i < constraints.size(); i++){
        for(size_t j = 0; j < constraints[i].size(); j++)
            if(constraints[i][j]->config.name == name)
                return true;
    }
    return false;
}

void WbcScene::sortConstraintConfig(const std::vector<ConstraintConfig>& config, std::vector< std::vector<ConstraintConfig> >& sorted_config){

    // Get highest prio
    int max_prio = 0;
    for(uint i = 0; i < config.size(); i++){

        if(config[i].priority > max_prio)
            max_prio = config[i].priority;
    }
    sorted_config.resize(max_prio + 1);

    for(uint i = 0; i < config.size(); i++)
        sorted_config[config[i].priority].push_back(config[i]);

    // Erase empty priorities
    for(uint i = 0; i < sorted_config.size(); i++){
        if(sorted_config[i].empty())
        {
            sorted_config.erase(sorted_config.begin() + i, sorted_config.begin() + i + 1);
            i--;
        }
    }
}

std::vector<int> WbcScene::getNConstraintVariablesPerPrio(const std::vector<ConstraintConfig> &config){

    std::vector< std::vector<ConstraintConfig> > sorted_config;
    sortConstraintConfig(config, sorted_config);

   std::vector<int> nn_pp(sorted_config.size());
    for(size_t i = 0; i < sorted_config.size(); i++){
        nn_pp[i] = 0;
        for(size_t j = 0; j < sorted_config[i].size(); j++)
            nn_pp[i] += sorted_config[i][j].noOfConstraintVariables();
    }
    return nn_pp;
}

} // namespace wbc