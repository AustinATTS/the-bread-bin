#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace breadbin::core {
    enum class TextFileType {
        Unknown,
        Loaf,
        Toml
    };

    struct TextFile {
        std::string contents;
        TextFileType type = TextFileType::Unknown;

        bool save_to_file(const std::filesystem::path& path) const;
        bool load_from_file(const std::filesystem::path& path);

        private:
            static TextFileType detect_type(const std::filesystem::path& path);
    };

} // namespace breadbin::core