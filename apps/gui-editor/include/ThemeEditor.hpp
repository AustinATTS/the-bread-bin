#pragma once
#include <breadbin/ThemeFile.hpp>
#include <breadbin/ReloadManager.hpp>
#include <filesystem>
#include <optional>
#include <map>
#include <imgui.h>
#include <vector>
#include <string>

namespace breadbin::gui {
    class ThemeEditor {
        public:
            ThemeEditor(bool& dirty_flag, breadbin::core::ThemeFile& active_theme, std::optional<std::filesystem::path>& theme_path, breadbin::core::ReloadManager& reload_mgr);

            bool is_active() const;
            bool is_dirty() const;

            void set_on_saved(std::function<void(const std::filesystem::path&)> cb);

            bool save();
            bool save_as();

            void render(bool* p_open, uint32_t dockspace_id);

        private:
            bool& m_dirty;
            std::function<void(const std::filesystem::path&)> m_on_saved;

            breadbin::core::ThemeFile& m_theme;
            std::optional<std::filesystem::path> m_path;
            breadbin::core::ReloadManager& m_reload_mgr;

            char m_name_buf[128] {};
    };
} // namespace breadbin::gui