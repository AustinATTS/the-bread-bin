#pragma once
#include <string>
#include <filesystem>
#include <vector>

namespace breadbin::gui {
    class TextEditor {
        public:
            TextEditor() = default;

            void open_file(const std::filesystem::path& path);
            void save_file();
            void render(bool* p_open);

            bool is_open() const {
                return m_is_open;
            }
            void set_open(bool open) {
                m_is_open = open;
            }

        private:
            std::filesystem::path m_current_path;
            std::string m_buffer;
            bool m_is_open = false;
            bool m_dirty = false;
    };
} // namespace breadbin::gui