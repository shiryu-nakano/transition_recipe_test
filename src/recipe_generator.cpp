#include "transition_recipe_test/recipe_generator.hpp"

namespace transition_recipe_test
{

    TransitionRecipe build_transition_recipe(
        const std::string &from_state,
        const std::string &target_state)
    {
        TransitionRecipe r;
        if(from_state == "ALL_UNCONFIGURED")
        {
            if (target_state == "STATE_ALL_OFF")
            {
                r.description = "ALL_UNCONFIGURED -> STATE_ALL_OFF: configure all nodes";

                ActionStep step1;
                step1.target_node_name = "A_node";
                step1.operation = "configure"; // A_node を INACTIVE にする
                step1.timeout_s = 3.0;
                step1.retry = 0;

                ActionStep step2;
                step2.target_node_name = "B_node";
                step2.operation = "configure"; // B_node を INACTIVE にする
                step2.timeout_s = 3.0;
                step2.retry = 0;

                ActionStep step3;
                step3.target_node_name = "C_node";
                step3.operation = "configure"; // C_node を INACTIVE にする
                step3.timeout_s = 3.0;
                step3.retry = 0;

                r.steps.push_back(step1);
                r.steps.push_back(step2);
                r.steps.push_back(step3);
                return r;
            }
        }

        if (from_state == "STATE_ALL_OFF")
        {
            if (target_state == "STATE_A_ONLY")
            {
                r.description = "STATE_ALL_OFF -> STATE_A_ONLY: activate only A_node";

                ActionStep step;
                step.target_node_name = "A_node";
                step.operation = "activate"; // A_node を ACTIVE にする
                step.timeout_s = 3.0;
                step.retry = 0;

                r.steps.push_back(step);
                return r;
            }
        }

        if (from_state == "STATE_A_ONLY")
        {
            if (target_state == "STATE_B_ONLY")
            {
                r.description = "STATE_A_ONLY -> STATE_B_ONLY: deactivate A_node, activate B_node";

                ActionStep step1;
                step1.target_node_name = "A_node";
                step1.operation = "deactivate"; // A_node を INACTIVE にする
                step1.timeout_s = 3.0;
                step1.retry = 0;

                ActionStep step2;
                step2.target_node_name = "B_node";
                step2.operation = "activate"; // B_node を ACTIVE にする
                step2.timeout_s = 3.0;
                step2.retry = 0;

                r.steps.push_back(step1);
                r.steps.push_back(step2);
                return r;
            }
            if (target_state == "STATE_C_ONLY")
            {
                r.description = "STATE_A_ONLY -> STATE_C_ONLY: deactivate A_node, activate C_node";

                ActionStep step1;
                step1.target_node_name = "A_node";
                step1.operation = "deactivate"; // A_node を INACTIVE にする
                step1.timeout_s = 3.0;
                step1.retry = 0;

                ActionStep step2;
                step2.target_node_name = "C_node";
                step2.operation = "activate"; // C_node を ACTIVE にする
                step2.timeout_s = 3.0;
                step2.retry = 0;

                r.steps.push_back(step1);
                r.steps.push_back(step2);
                return r;
            }
            if (target_state == "STATE_ALL_OFF")
            {
                r.description = "STATE_A_ONLY -> STATE_ALL_OFF: deactivate A_node";

                ActionStep step;
                step.target_node_name = "A_node";
                step.operation = "deactivate"; // A_node を INACTIVE にする
                step.timeout_s = 3.0;
                step.retry = 0;

                r.steps.push_back(step);
                return r;
            }
        }

        if(from_state == "STATE_B_ONLY")
        {
            if (target_state == "STATE_C_ONLY")
            {
                r.description = "STATE_B_ONLY -> STATE_C_ONLY: deactivate B_node, activate C_node";

                ActionStep step1;
                step1.target_node_name = "B_node";
                step1.operation = "deactivate"; // B_node を INACTIVE にする
                step1.timeout_s = 3.0;
                step1.retry = 0;

                ActionStep step2;
                step2.target_node_name = "C_node";
                step2.operation = "activate"; // C_node を ACTIVE にする
                step2.timeout_s = 3.0;
                step2.retry = 0;

                r.steps.push_back(step1);
                r.steps.push_back(step2);
                return r;
            }
            if( target_state == "STATE_A_ONLY")
            {
                r.description = "STATE_B_ONLY -> STATE_A_ONLY: deactivate B_node, activate A_node";

                ActionStep step1;
                step1.target_node_name = "B_node";
                step1.operation = "deactivate"; // B_node を INACTIVE にする
                step1.timeout_s = 3.0;
                step1.retry = 0;

                ActionStep step2;
                step2.target_node_name = "A_node";
                step2.operation = "activate"; // A_node を ACTIVE にする
                step2.timeout_s = 3.0;
                step2.retry = 0;

                r.steps.push_back(step1);
                r.steps.push_back(step2);
                return r;
            }
        }
        
        if(from_state == "STATE_C_ONLY")
        {
            if (target_state == "STATE_ALL_OFF")
            {
                r.description = "STATE_C_ONLY -> STATE_ALL_OFF: deactivate C_node";

                ActionStep step;
                step.target_node_name = "C_node";
                step.operation = "deactivate"; // C_node を INACTIVE にする
                step.timeout_s = 3.0;
                step.retry = 0;

                r.steps.push_back(step);
                return r;
            }
        }
        // ================================
        // それ以外の組み合わせは未実装
        // ================================
        r.description = "No TransitionRecipe defined for: " + from_state + " -> " + target_state;
        // steps は空のまま（何もしないレシピ）
        return r;
    }

} // namespace transition_recipe_test
