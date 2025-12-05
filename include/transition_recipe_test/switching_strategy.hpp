// transition_recipe_test/switching_strategy.hpp
#pragma once

#include <optional>
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
    ///
    /// @return 遷移が発火した場合は target_state を返し、遷移なしなら std::nullopt。
    std::optional<std::string> decide_next_state(
        const std::string &current_state_id,
        int &phase,
        double since_last,
        double x,
        double y) const;
};

} // namespace transition_recipe_test
