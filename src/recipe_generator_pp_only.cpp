// src/recipe_generator_pp_only.cpp
//
// pure_pursuit_planner のみを管理対象とする構成（config/pp_only.yaml）用の recipe 定義。
// 状態名は dwa_pp.yaml と同じだが、DWA の手順を含まない。
// manager の recipe_set:=pp_only のときに使われる。
//
// TODO: 管理対象ノードと状態グラフから recipe を自動生成する（今はハードコード）

#include "transition_recipe_test/recipe_generator.hpp"

namespace transition_recipe_test
{
    namespace
    {
        constexpr double kDefaultTimeoutSec = 3.0;

        ActionStep make_step(const std::string &node_name, const std::string &op)
        {
            ActionStep s;
            s.target_node_name = node_name;
            s.operation = op;
            s.timeout_s = kDefaultTimeoutSec;
            s.retry = 0;
            return s;
        }
    } // namespace

    TransitionRecipe build_transition_recipe_pp_only(
        const std::string &from_state,
        const std::string &target_state)
    {
        TransitionRecipe r;

        // ========================================
        // ALL_UNCONFIGURED -> ALL_CONFIGURED
        //   pp を configure（INACTIVE に持っていく）
        // ========================================
        if (from_state == "ALL_UNCONFIGURED" && target_state == "ALL_CONFIGURED")
        {
            r.description = "ALL_UNCONFIGURED -> ALL_CONFIGURED: configure pp (pp_only)";
            r.steps.push_back(make_step("pure_pursuit_planner", "configure"));
            return r;
        }

        // ========================================
        // ALL_CONFIGURED -> pure_pursuit_planner
        //   pp を activate
        // ========================================
        if (from_state == "ALL_CONFIGURED" && target_state == "pure_pursuit_planner")
        {
            r.description = "ALL_CONFIGURED -> pure_pursuit_planner: activate pp (pp_only)";
            r.steps.push_back(make_step("pure_pursuit_planner", "activate"));
            return r;
        }

        // ========================================
        // pure_pursuit_planner -> ALL_CONFIGURED
        //   pp を deactivate（停止）
        // ========================================
        if (from_state == "pure_pursuit_planner" && target_state == "ALL_CONFIGURED")
        {
            r.description = "pure_pursuit_planner -> ALL_CONFIGURED: deactivate pp (pp_only)";
            r.steps.push_back(make_step("pure_pursuit_planner", "deactivate"));
            return r;
        }

        // ========================================
        // それ以外は未定義（空 recipe）
        // ========================================
        r.description = "No TransitionRecipe defined (pp_only) for: " + from_state + " -> " + target_state;
        return r;
    }

} // namespace transition_recipe_test
