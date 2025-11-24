#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include "transition_recipe_test/common_types.hpp"  // SemanticState

namespace transition_recipe_test
{

class Graph
{
public:
    Graph();
    explicit Graph(std::unordered_map<std::string, SemanticState> state_dictionary);

    // 全体辞書を差し替える
    void setStateDictionary(std::unordered_map<std::string, SemanticState> state_dictionary);

    // 現在の SemanticState に一致する state_id を返す
    std::optional<std::string> getCurrentSemanticState(
        const SemanticState & semantic) const noexcept;
    
    // 辞書のサイズ取得
    std::size_t size() const noexcept;

private:
    std::unordered_map<std::string, SemanticState> state_dictionary_;
};

} // namespace transition_recipe_test
