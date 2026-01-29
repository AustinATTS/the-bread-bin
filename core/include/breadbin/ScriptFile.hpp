#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace breadbin::core {
    enum class ScriptFileType {
        Unknown,
        Loaf,
        Toml
    };

    struct ScriptFile {
        std::string contents;
        ScriptFileType type = ScriptFileType::Unknown;

        bool save_to_file(const std::filesystem::path& path) const;
        bool load_from_file(const std::filesystem::path& path);

    private:
        static ScriptFileType detect_type(const std::filesystem::path& path);
    };

} // namespace breadbin::core