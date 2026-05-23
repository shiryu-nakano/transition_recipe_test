// include/transition_judge_node/graph/graph_generator.hpp
#pragma once

#include <string>
#include "transition_recipe_test/graph/graph.hpp"
#include "transition_recipe_test/common_types.hpp"
#include <yaml-cpp/yaml.h>

namespace transition_judge_node {
Graph init_state_graph();
Graph init_state_graph_from_yaml(const std::string &yaml_path);
}
