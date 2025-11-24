// src/graph_generator.cpp
#include "transition_recipe_test/graph_generator.hpp"

namespace transition_recipe_test
{
    namespace
    {
        // 文字列 → SemanticState::State 変換ヘルパ
        SemanticState::State parse_state_enum(const std::string &s)
        {
            using S = SemanticState::State;
            if (s == "UNCONFIGURED")
                return S::UNCONFIGURED;
            if (s == "INACTIVE")
                return S::INACTIVE;
            if (s == "ACTIVE")
                return S::ACTIVE;
            if (s == "FINALIZED")
                return S::FINALIZED;
            if (s == "UNKNOWN")
                return S::UNKNOWN;

            // 想定外の文字列が来たときは例外 or デフォルト UNKNOWN
            throw std::runtime_error("Unknown SemanticState string: " + s);
            // return S::UNKNOWN;
        }
    } // namespace
    Graph init_state_graph()
    {
        std::unordered_map<std::string, SemanticState> dict;

        // Helper lambda for readability
        auto make = [&](SemanticState::State a, SemanticState::State b, SemanticState::State c)
        {
            SemanticState s;
            s.node_states["A_node"] = a;
            s.node_states["B_node"] = b;
            s.node_states["C_node"] = c;
            return s;
        };

        using S = SemanticState::State;

        // - - -
        dict["ALL_UNCONFIGURED"] = make(S::UNCONFIGURED, S::UNCONFIGURED, S::UNCONFIGURED);

        // 0 0 0
        dict["STATE_ALL_OFF"] = make(S::INACTIVE, S::INACTIVE, S::INACTIVE);

        // 1 0 0
        dict["STATE_A_ONLY"] = make(S::ACTIVE, S::INACTIVE, S::INACTIVE);

        // 0 1 0
        dict["STATE_B_ONLY"] = make(S::INACTIVE, S::ACTIVE, S::INACTIVE);

        // 0 0 1
        dict["STATE_C_ONLY"] = make(S::INACTIVE, S::INACTIVE, S::ACTIVE);

        // 1 1 0
        dict["STATE_AB_ON"] = make(S::ACTIVE, S::ACTIVE, S::INACTIVE);

        // 1 0 1
        dict["STATE_AC_ON"] = make(S::ACTIVE, S::INACTIVE, S::ACTIVE);

        // 0 1 1
        dict["STATE_BC_ON"] = make(S::INACTIVE, S::ACTIVE, S::ACTIVE);

        // 1 1 1
        dict["STATE_ALL_ON"] = make(S::ACTIVE, S::ACTIVE, S::ACTIVE);

        // 完成
        return Graph(std::move(dict));
    }

    Graph init_state_graph_from_yaml(const std::string &yaml_path)
    {
        YAML::Node root = YAML::LoadFile(yaml_path);

        if (!root["states"] || !root["states"].IsSequence())
        {
            throw std::runtime_error("YAML error: 'states' sequence is missing in " + yaml_path);
        }

        std::unordered_map<std::string, SemanticState> dict;

        for (const auto &state_node : root["states"])
        {
            // 1. id の取得
            if (!state_node["id"])
            {
                throw std::runtime_error("YAML error: state entry missing 'id'");
            }
            std::string id = state_node["id"].as<std::string>();

            // 2. nodes の取得
            if (!state_node["nodes"] || !state_node["nodes"].IsMap())
            {
                throw std::runtime_error("YAML error: state '" + id + "' missing 'nodes' map");
            }

            SemanticState semantic;
            const auto &nodes_map = state_node["nodes"];

            for (auto it = nodes_map.begin(); it != nodes_map.end(); ++it)
            {
                std::string node_name = it->first.as<std::string>();
                std::string state_str = it->second.as<std::string>();

                semantic.node_states[node_name] = parse_state_enum(state_str);
            }

            dict.emplace(id, std::move(semantic));
        }

        return Graph(std::move(dict));
    }
}
