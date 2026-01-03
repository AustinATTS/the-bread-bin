#pragma once
#include <vector>
#include <string>
#include <filesystem>

namespace breadbin::gui {
    class LoafBrowser {
        public:
            LoafBrowser();
            void render();

        private:
            void refresh_loafs();
            std::vector<std::filesystem::path> loaf_files_;
            std::filesystem::path config_path_;
    };
} // namespace breadbin::gui