// src/recipe_generator.cpp
//
// transition_judge_interface との連携を前提とした recipe 定義。
// 対象 lifecycle ノード： dwa_planner / pure_pursuit_planner
//
// 旧 A/B/C 用の demo 実装は src/sample/recipe_generator.cpp を参照（こちらは link しない）。

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

    TransitionRecipe build_transition_recipe(
        const std::string &from_state,
        const std::string &target_state)
    {
        TransitionRecipe r;

        // ========================================
        // ALL_UNCONFIGURED -> ALL_CONFIGURED
        //   両ノードを configure（INACTIVE に持っていく）
        // ========================================
        if (from_state == "ALL_UNCONFIGURED" && target_state == "ALL_CONFIGURED")
        {
            r.description = "ALL_UNCONFIGURED -> ALL_CONFIGURED: configure both planners";
            r.steps.push_back(make_step("dwa_planner", "configure"));
            r.steps.push_back(make_step("pure_pursuit_planner", "configure"));
            return r;
        }

        // ========================================
        // ALL_CONFIGURED -> pure_pursuit_planner
        //   pp を activate（dwa は INACTIVE のまま）
        // ========================================
        if (from_state == "ALL_CONFIGURED" && target_state == "pure_pursuit_planner")
        {
            r.description = "ALL_CONFIGURED -> pure_pursuit_planner: activate pp";
            r.steps.push_back(make_step("pure_pursuit_planner", "activate"));
            return r;
        }

        // ========================================
        // ALL_CONFIGURED -> dwa_planner
        //   dwa を activate（pp は INACTIVE のまま）
        // ========================================
        if (from_state == "ALL_CONFIGURED" && target_state == "dwa_planner")
        {
            r.description = "ALL_CONFIGURED -> dwa_planner: activate dwa";
            r.steps.push_back(make_step("dwa_planner", "activate"));
            return r;
        }

        // ========================================
        // pure_pursuit_planner -> dwa_planner
        //   pp を deactivate、続けて dwa を activate
        // ========================================
        if (from_state == "pure_pursuit_planner" && target_state == "dwa_planner")
        {
            r.description = "pure_pursuit_planner -> dwa_planner: switch pp -> dwa";
            r.steps.push_back(make_step("pure_pursuit_planner", "deactivate"));
            r.steps.push_back(make_step("dwa_planner", "activate"));
            return r;
        }

        // ========================================
        // dwa_planner -> pure_pursuit_planner
        //   dwa を deactivate、続けて pp を activate
        // ========================================
        if (from_state == "dwa_planner" && target_state == "pure_pursuit_planner")
        {
            r.description = "dwa_planner -> pure_pursuit_planner: switch dwa -> pp";
            r.steps.push_back(make_step("dwa_planner", "deactivate"));
            r.steps.push_back(make_step("pure_pursuit_planner", "activate"));
            return r;
        }

        // ========================================
        // 任意のアクティブ状態 -> ALL_CONFIGURED
        //   現状アクティブな側を deactivate
        // ========================================
        if (target_state == "ALL_CONFIGURED")
        {
            if (from_state == "pure_pursuit_planner")
            {
                r.description = "pure_pursuit_planner -> ALL_CONFIGURED: deactivate pp";
                r.steps.push_back(make_step("pure_pursuit_planner", "deactivate"));
                return r;
            }
            if (from_state == "dwa_planner")
            {
                r.description = "dwa_planner -> ALL_CONFIGURED: deactivate dwa";
                r.steps.push_back(make_step("dwa_planner", "deactivate"));
                return r;
            }
        }

        // ========================================
        // それ以外は未定義（空 recipe）
        // ========================================
        r.description = "No TransitionRecipe defined for: " + from_state + " -> " + target_state;
        return r;
    }

} // namespace transition_recipe_test
