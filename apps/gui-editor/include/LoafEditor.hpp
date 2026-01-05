#pragma once
#include <breadbin/LoafFile.hpp>
#include <breadbin/ReloadManager.hpp>
#include <filesystem>
#include <optional>
#include <map>
#include <imgui.h>
#include <vector>
#include <string>

namespace breadbin::gui {
    class LoafEditor {
        public:
            LoafEditor(bool& dirty_flag, breadbin::core::LoafFile& active_loaf, std::optional<std::filesystem::path>& loaf_path, breadbin::core::ReloadManager& reload_mgr);

            bool is_active() const;
            bool is_dirty() const;

            void set_on_saved(std::function<void(const std::filesystem::path&)> cb);


            bool save();
            bool save_as();

            void refresh_installed_apps();
            void update_apps(const std::map<std::string, std::string>& apps, const std::vector<std::string>& app_names);

            void render(bool* p_open, uint32_t dockspace_id);

        private:
            void render_actions();

            bool& m_dirty;
            std::function<void(const std::filesystem::path&)> m_on_saved;

            breadbin::core::LoafFile& m_loaf;
            std::optional<std::filesystem::path>& m_path;
            breadbin::core::ReloadManager& m_reload_mgr;

            char m_name_buf[128]{};

            std::map<std::string, std::string> m_installed_apps;
            std::vector<std::string> m_app_names;
    };

} // namespace breadbin::gui
