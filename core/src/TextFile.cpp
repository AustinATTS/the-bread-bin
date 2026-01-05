#include <breadbin/TextFile.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

namespace breadbin::core {

    TextFileType TextFile::detect_type(const std::filesystem::path& path) {
        if (path.extension() == ".loaf") return TextFileType::Loaf;
        if (path.extension() == ".toml") return TextFileType::Toml;
        return TextFileType::Unknown;
    }

    bool TextFile::load_from_file(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            std::cerr << "Text file does not exist: " << path << '\n';
            return false;
        }

        std::ifstream file(path, std::ios::in);
        if (!file.is_open()) {
            std::cerr << "Failed to open text file: " << path << '\n';
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        contents = buffer.str();
        type = detect_type(path);

        return true;
    }

    bool TextFile::save_to_file(const std::filesystem::path& path) const {
        std::ofstream file(path, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "Failed to save text file: " << path << '\n';
            return false;
        }

        file << contents;
        return true;
    }

} // namespace breadbin::core
