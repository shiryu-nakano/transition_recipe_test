#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <cstdint>

namespace transition_recipe_test
{

struct TransitionContext
{
    std::string current_state;
    bool obstacle_detected;
    double obstacle_distance;
    double elapsed_time;
    double current_x;
    double current_y;
};

struct SemanticState
{
    enum class State : std::uint8_t
    {
        UNKNOWN = 0,
        UNCONFIGURED = 1,
        INACTIVE = 2,
        ACTIVE = 3,
        FINALIZED = 4
    };

    std::unordered_map<std::string, State> node_states;

    bool operator==(const SemanticState &other) const noexcept
    {
        return node_states == other.node_states;
    }
};

struct ActionStep
{
    std::string target_node_name;
    std::string operation;
    double timeout_s{5.0};
    int retry{0};
};

struct TransitionRecipe
{
    std::vector<ActionStep> steps;
    std::string description;
};

} // namespace transition_recipe_test
