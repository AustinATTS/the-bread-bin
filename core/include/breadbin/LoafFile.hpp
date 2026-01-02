#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace breadbin::core {
    enum class ActionType {
        App,
        File,
        Link
    };

    struct Action {
        ActionType type;
        std::string target;
        std::vector<std::string> args;
    };

    struct LoafFile {
        std::string app_name;
        std::vector <Action> actions;

        // Generic function to simulate use
        std::string get_summary() const;

        bool save_to_file(const std::filesystem::path& path) const;
        bool load_from_file(const std::filesystem::path& path);
    };
} // namespace breadbin::core