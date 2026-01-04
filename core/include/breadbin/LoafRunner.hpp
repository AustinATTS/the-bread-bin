#pragma once
#include <breadbin/LoafFile.hpp>

namespace breadbin::core {
    class LoafRunner {
        public:
            static void execute(const LoafFile& loaf);

        private:
            static void launch_app(const std::string& bin, const std::vector<std::string>& args);
            static void launch_generic(const std::string& target);
    };
} // namespace breadbin::core