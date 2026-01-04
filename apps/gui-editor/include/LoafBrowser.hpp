#pragma once
#include <breadbin/LoafFile.hpp>
#include <TextEditor.hpp>
#include <vector>
#include <filesystem>
#include <optional>

namespace breadbin::gui {
    class LoafBrowser {
        public:
            LoafBrowser(bool& dirty_flag, core::LoafFile& active_loaf, TextEditor& editor, std::optional<std::filesystem::path>& active_path);
            void render(bool* p_open);
            void reload();

        private:
            void refresh_files();
            void handle_file_action(const std::filesystem::path& path);

            bool& m_dirty;
            core::LoafFile& m_active_loaf;
            TextEditor& m_editor;
            std::optional<std::filesystem::path>& m_active_path;

            std::filesystem::path m_root_path;
            std::filesystem::path m_selected_path;
            std::vector<std::filesystem::path> m_files;

            bool m_show_unsaved_modal = false;
            bool m_show_delete_modal = false;
    };
} // namespace breadbin::gui