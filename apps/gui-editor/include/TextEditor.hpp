#pragma once
#include <breadbin/TextFile.hpp>
#include <breadbin/ReloadManager.hpp>
#include <filesystem>
#include <optional>
#include <imgui.h>
#include <vector>
#include <string>

namespace breadbin::gui {
    class TextEditor {
        public:
            TextEditor(bool& dirty_flag, breadbin::core::TextFile& active_file, std::optional<std::filesystem::path>& file_path,breadbin::core::ReloadManager& reload_mgr);

            bool is_active() const;
            bool is_dirty() const;

            bool save();
            bool save_as();

            void open_file(const std::filesystem::path& path);



            void render(bool* p_open, uint32_t dockspace_id);

        private:
            std::optional<std::filesystem::path> m_last_loaded_path;

            bool m_dirty = false;
            breadbin::core::TextFile m_file;
            std::optional<std::filesystem::path> m_path;
            breadbin::core::ReloadManager& m_reload_mgr;

        char m_name_buf[128] {};



    };

} // namespace breadbin::gui
