#pragma once
#include <breadbin/LoafFile.hpp>
#include <filesystem>
#include <string>
#include <map>
#include <vector>

namespace breadbin::gui {
    class LoafEditor {
        public:
            LoafEditor(bool& dirty_flag, breadbin::core::LoafFile& active_loaf);

            void render(bool* p_open, uint32_t dockspace_id);
            void update_apps(const std::map<std::string, std::string>& apps, const std::vector<std::string>& app_names);

        private:
            bool& m_dirty;
            breadbin::core::LoafFile& m_loaf;
            char m_name_buf[128];

            std::map<std::string, std::string> m_installed_apps;
            std::vector<std::string> m_app_names;

            void render_actions();
    };
} // namespace breadbin::gui