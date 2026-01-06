#include <fstream>
#include <breadbin/ThemeFile.hpp>
#include <toml++/toml.h>
#include <filesystem>
#include <iostream>

namespace breadbin::core {
    bool ThemeFile::load_from_file(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            std::cerr << "Theme file does not exist: " << path << std::endl;
            return false;
        }

        try {
            theme = toml::parse_file(path.string());

            if (auto n = theme["name"].value<std::string>()) {
                name = *n;
            }

            return true;
        }
        catch (const toml::parse_error& e) {
            std::cerr << "Failed to load theme file: " << e.what() << std::endl;
            return false;
        }
    }

    bool ThemeFile::save_to_file(const std::filesystem::path& path) const {
        std::ofstream fout(path);
        if (!fout.is_open()) {
            return false;
        }

        if (!name.empty()) {
            auto copy = theme;
            copy.insert_or_assign("name", name);
            fout << copy;
        }
        else {

            fout << theme;
        }

        fout.close();
        return true;
    }
} // namespace breadbin::core