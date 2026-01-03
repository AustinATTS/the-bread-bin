#include <fstream>
#include <breadbin/LoafFile.hpp>
#include <yaml-cpp/yaml.h>
#include <sstream>

namespace breadbin::core {
    bool LoafFile::save_to_file(const std::filesystem::path& path) const {
        YAML::Emitter out;

        out << YAML::BeginMap;
        out << YAML::Key << "name" << YAML::Value << name;

        out << YAML::Key << "actions" << YAML::Value << YAML::BeginSeq;
        for (const auto& action : actions) {
            out << YAML::BeginMap;

            out << YAML::Key << "type";
            switch (action.type) {
                case ActionType::App: out << "app"; break;
                case ActionType::File: out << "file"; break;
                case ActionType::Link: out << "link"; break;
            }

            out << YAML::Key << "target" << YAML::Value << action.target;

            out << YAML::Key << "args" << YAML::Value << YAML::BeginSeq;
            for (const auto& arg : action.args) {
                out << arg;
            }
            out << YAML::EndSeq;

            out << YAML::EndMap;
        }
        out << YAML::EndSeq;

        out << YAML::EndMap;

        std::ofstream fout(path);
        if (!fout.is_open()) {
            return false;
        }

        fout << out.c_str();
        fout.close();
        return true;
    }

    bool LoafFile::load_from_file(const std::filesystem::path& path) {

    }
} // namespace breadbin::core