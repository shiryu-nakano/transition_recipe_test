#pragma once

#include <string>
#include "transition_recipe_test/common_types.hpp"

namespace transition_recipe_test
{

    TransitionRecipe build_transition_recipe(
        const std::string &from_state,
        const std::string &target_state);

} // namespace transition_recipe_test
