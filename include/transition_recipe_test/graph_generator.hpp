// include/transition_recipe_test/graph_generator.hpp
#pragma once

#include <string>
#include "transition_recipe_test/graph.hpp"
#include "transition_recipe_test/common_types.hpp"
#include <yaml-cpp/yaml.h>

namespace transition_recipe_test {
Graph init_state_graph();
Graph init_state_graph_from_yaml(const std::string &yaml_path);
}
