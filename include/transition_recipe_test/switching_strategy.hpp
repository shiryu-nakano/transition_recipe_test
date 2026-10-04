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
    /// 使う recipe の組を選ぶ。"dwa_pp"（既定）または "pp_only"。
    /// TODO: 管理対象ノードと状態グラフから recipe を自動生成する（今はハードコード）
    void set_recipe_set(const std::string &recipe_set) { recipe_set_ = recipe_set; }
    const std::string &recipe_set() const { return recipe_set_; }

    /// (from, to) から対応する TransitionRecipe を引き当てる lookup。
    /// 判定は外部ノードが行う前提で、本クラスは recipe 取得のみを担う。
    /// 未定義の組み合わせなら std::nullopt。
    std::optional<TransitionRecipe> call_transition_recipe(
        const std::string &from_state_id,
        const std::string &target_state_id) const;

    /// 状態遷移の判定だけを行う純粋ロジック
    ///
    /// @param current_state_id  Graph から得られた現在のシステム状態ID
    /// @param phase             状態遷移判定の戦略切り替えフラグ
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

private:
    std::string recipe_set_ = "dwa_pp";
};

} // namespace transition_recipe_test
