// transition_recipe_test/switching_strategy.cpp
#include "transition_recipe_test/switching_strategy.hpp"

#include <cmath>    // std::hypot

namespace transition_recipe_test
{

std::optional<std::string> SwitchingStrategy::decide_next_state(
    const std::string &current_state_id,
    int &phase,
    double since_last,
    double x,
    double y) const
{
    (void)current_state_id;  // 現状は使っていないが、将来ロジックに含めるかもしれないので残しておく

    // フェーズ0: 起動から5秒後に STATE_ALL_OFF
    if (phase == 0 && since_last >= 5.0)
    {
        phase = 1;
        return std::string{"STATE_ALL_OFF"};
    }

    // フェーズ1: さらに5秒後に STATE_A_ONLY
    if (phase == 1 && since_last >= 5.0)
    {
        phase = 2;
        return std::string{"STATE_A_ONLY"};
    }

    // フェーズ2: (20,0) に近づいたら STATE_B_ONLY
    if (phase == 2)
    {
        double dist = std::hypot(x - 20.0, y - 0.0);
        if (dist < 1.0)
        {
            phase = 3;
            return std::string{"STATE_B_ONLY"};
        }
    }

    // フェーズ3: (40,0) に近づいたら STATE_C_ONLY
    if (phase == 3)
    {
        double dist = std::hypot(x - 40.0, y - 0.0);
        if (dist < 1.0)
        {
            phase = 4;  // 完了フェーズ
            return std::string{"STATE_C_ONLY"};
        }
    }

    // 遷移なし
    return std::nullopt;
}

} // namespace transition_recipe_test
