#pragma once
#include <filesystem>
#include <string>
#include <toml++/toml.h>

namespace breadbin::core {
    struct ThemeFile {
        std::string name;

        toml::table theme;

        bool save_to_file(const std::filesystem::path& path) const;
        bool load_from_file(const std::filesystem::path& path);
    };
} // namespace breadbin::core