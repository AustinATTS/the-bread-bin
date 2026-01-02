#pragma once
#include <string>
#include <vector>

namespace breadbin::core {
    struct LoafFile {
        std::string app_name;
        std::vector<std::string> args;

        // Generic function to simulate use
        std::string get_summary() const;
    };
} // namespace breadbin::core