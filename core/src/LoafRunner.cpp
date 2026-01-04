#include <breadbin/LoafRunner.hpp>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>

namespace breadbin::core {
    void LoafRunner::execute(const LoafFile& loaf) {
        for (const auto& action : loaf.actions) {
            switch (action.type) {
            case ActionType::App:
                launch_app(action.target, action.args);
                break;
            case ActionType::Link:
            case ActionType::File:
                launch_generic(action.target);
                break;
            }
        }
    }

    void LoafRunner::launch_app(const std::string& bin, const std::vector<std::string>& args) {
        auto clean_path = [](std::string s) {
            if (s.size() > 2 && s.front() == '"' && s.back() == '"') {
                return s.substr(1, s.size() - 2);
            }
            return s;
        };

        std::string final_bin = clean_path(bin);

        pid_t pid = fork();

        if (pid == 0) {
            std::vector<char*> c_args;
            c_args.push_back(const_cast<char*>(final_bin.c_str()));
            for (const auto& arg : args) {
                c_args.push_back(const_cast<char*>(arg.c_str()));
            }
            c_args.push_back(nullptr);

            execvp(c_args[0], c_args.data());

            std::cerr << "Failed to launch: " << bin << std::endl;
            _exit(1);
        }
        else {
            if (pid < 0) {
                perror("fork");
            }
        }
    }

    void LoafRunner::launch_generic(const std::string& target) {
        std::string command = "xdg-open \"" + target + "\" &";
        if (std::system(command.c_str()) != 0) {
            std::cerr << "Failed to open target: " << target << std::endl;
        }
    }
} // namespace breadbin::core
