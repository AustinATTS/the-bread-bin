#include <fstream>
#include <breadbin/LoafFile.hpp>
#include <yaml-cpp/yaml.h>
#include <sstream>

namespace breadbin::core {
    std::string LoafFile::get_summary() const {
        std::stringstream ss;
        ss << "App: " << app_name << " (" << args.size() << " arguments) ";
        return ss.str();
    }

    bool LoafFile::save_to_file(const std::filesystem::path& path) const {
        YAML::Emitter out;

        out << YAML::BeginMap;
        out << YAML::Key << "app_name" << YAML::Value << app_name;
        out << YAML::Key << "args" << YAML::Value << YAML::BeginSeq;
        for (const auto& arg : args) {
            out << arg;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(path);
        if (!fout.is_open()) {
            return false;
        }

        fout << out.c_str();
        return true;
    }

    bool LoafFile::load_from_file(const std::filesystem::path& path) {

    }
} // namespace breadbin::core