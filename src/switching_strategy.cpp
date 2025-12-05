// transition_recipe_test/switching_strategy.cpp
#include "transition_recipe_test/switching_strategy.hpp"

namespace transition_recipe_test
{

std::optional<TransitionRecipe> SwitchingStrategy::decide_next_state(
    const std::string &current_state_id,
    int &phase,
    double since_last,
    double x,
    double y,
    std::string &out_target_state) const
{
    // 現状は current_state_id 自体は判定に使っていないが、
    // 将来的に条件に入れるかもしれないので受け取っておく。

    // フェーズ0: 起動から5秒後に STATE_ALL_OFF
    if (phase == 0 && since_last >= 5.0)
    {
        phase = 1;
        out_target_state = "STATE_ALL_OFF";
        return build_transition_recipe(current_state_id, out_target_state);
    }

    // フェーズ1: さらに5秒後に STATE_A_ONLY
    if (phase == 1 && since_last >= 5.0)
    {
        phase = 2;
        out_target_state = "STATE_A_ONLY";
        return build_transition_recipe(current_state_id, out_target_state);
    }

    // フェーズ2: (20,0) に近づいたら STATE_B_ONLY
    if (phase == 2)
    {
        double dist = std::hypot(x - 15.0, y - 0.0);
        if (dist < 1.0)
        {
            phase = 3;
            out_target_state = "STATE_B_ONLY";
            return build_transition_recipe(current_state_id, out_target_state);
        }
    }

    // フェーズ3: (40,0) に近づいたら STATE_C_ONLY
    if (phase == 3)
    {
        double dist = std::hypot(x - 30.0, y - 0.0);
        if (dist < 1.0)
        {
            phase = 4;  // 完了フェーズ
            out_target_state = "STATE_C_ONLY";
            return build_transition_recipe(current_state_id, out_target_state);
        }
    }

    // 遷移なし
    return std::nullopt;
}

} // namespace transition_recipe_test
