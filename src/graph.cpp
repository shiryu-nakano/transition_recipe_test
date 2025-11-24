#include "transition_recipe_test/graph.hpp"

namespace transition_recipe_test
{

    Graph::Graph() = default;

    Graph::Graph(std::unordered_map<std::string, SemanticState> state_dictionary)
        : state_dictionary_(std::move(state_dictionary))
    {
    }

    void Graph::setStateDictionary(std::unordered_map<std::string, SemanticState> state_dictionary)
    {
        state_dictionary_ = std::move(state_dictionary);
    }

    std::optional<std::string> Graph::getCurrentSemanticState(
        const SemanticState &semantic) const noexcept
    {
        for (const auto &[state_id, state_semantic] : state_dictionary_)
        {
            if (state_semantic == semantic)
            {
                return state_id;
            }
        }
        return std::nullopt;
    }
    std::size_t Graph::size() const noexcept
    {
        return state_dictionary_.size();
    }

} // namespace transition_recipe_test
