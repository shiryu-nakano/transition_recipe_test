#pragma once

#include <string>
#include "transition_recipe_test/common_types.hpp"

namespace transition_judge_node
{

    /// state_id ペアから対応する TransitionRecipe を構築する。
    ///
    /// 未定義の組み合わせに対しては description のみ設定された
    /// 空レシピ（steps が空）を返す。呼び出し側で steps.empty() を
    /// 判定して扱うこと。
    ///
    /// @param from_state    遷移元の state_id（例: "STATE_ALL_OFF"）
    /// @param target_state  遷移先の state_id（例: "STATE_A_ONLY"）
    /// @return 対応する TransitionRecipe。未定義時は空レシピ。
    TransitionRecipe build_transition_recipe(
        const std::string &from_state,
        const std::string &target_state);

} // namespace transition_judge_node
