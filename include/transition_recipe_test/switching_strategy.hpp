// transition_recipe_test/switching_strategy.hpp
#pragma once

#include "transition_recipe_test/recipe_generator.hpp"
#include "transition_recipe_test/common_types.hpp"

#include <optional>
#include <cmath>    // std::hypot
#include <string>

namespace transition_recipe_test {

class SwitchingStrategy
{
public:
    /// 状態遷移の判定だけを行う純粋ロジック
    ///
    /// @param current_state_id  Graph から得られた現在のシステム状態ID
    /// @param phase             フェーズカウンタ（元の temp_count_）。遷移したら内部で更新される（in/out）
    /// @param since_last        前回遷移からの経過秒数
    /// @param x                 現在位置 x
    /// @param y                 現在位置 y
    /// @param out_target_state  遷移が発火した場合に、ここにターゲット状態名を書き込む
    ///
    /// @return 遷移が発火した場合は TransitionRecipe を返し、
    ///         遷移なしなら std::nullopt。
    std::optional<TransitionRecipe> decide_next_state(
        const std::string &current_state_id,
        int &phase,
        double since_last,
        double x,
        double y,
        std::string &out_target_state) const;
};

} // namespace transition_recipe_test
