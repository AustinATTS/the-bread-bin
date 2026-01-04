#include <fstream>
#include <breadbin/LoafFile.hpp>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <iostream>

namespace breadbin::core {
    bool LoafFile::load_from_file(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            std::cerr << "Loaf file does not exist: " << path << std::endl;
            return false;
        }

        try {
            YAML::Node root = YAML::LoadFile(path.string());

            if (root["name"]) name = root["name"].as<std::string>();

            actions.clear();
            if (root["actions"]) {
                for (const auto& actionNode : root["actions"]) {
                    Action action;

                    std::string typeStr = actionNode["type"].as<std::string>();
                    if (typeStr == "app") action.type = ActionType::App;
                    else if (typeStr == "file") action.type = ActionType::File;
                    else if (typeStr == "link") action.type = ActionType::Link;
                    else action.type = ActionType::App;

                    if (actionNode["target"]) action.target = actionNode["target"].as<std::string>();

                    action.args.clear();
                    if (actionNode["args"]) {
                        for (const auto& argNode : actionNode["args"]) {
                            action.args.push_back(argNode.as<std::string>());
                        }

                        actions.push_back(std::move(action));
                    }
                }
                return true;
            }
        } catch (const YAML::Exception& e) {
            std::cerr << "Failed to load loaf file: " << e.what() << std::endl;
            return false;
        }
    }

    bool LoafFile::save_to_file(const std::filesystem::path& path) const {
        YAML::Emitter out;

        out << YAML::BeginMap;
        out << YAML::Key << "name" << YAML::Value << name;

        out << YAML::Key << "actions" << YAML::Value << YAML::BeginSeq;
        for (const auto& action : actions) {
            out << YAML::BeginMap;

            out << YAML::Key << "type";
            switch (action.type) {
                case ActionType::App:
                    out << "app";
                    break;
                case ActionType::File:
                    out << "file";
                    break;
                case ActionType::Link:
                    out << "link";
                    break;
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
} // namespace breadbin::core