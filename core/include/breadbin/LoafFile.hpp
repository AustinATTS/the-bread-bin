#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace breadbin::core {
    struct LoafFile {
        std::string app_name;
        std::vector<std::string> args;

        // Generic function to simulate use
        std::string get_summary() const;

        bool save_to_file(const std::filesystem::path& path) const;
        bool load_from_file(const std::filesystem::path& path);
    };
} // namespace breadbin::core