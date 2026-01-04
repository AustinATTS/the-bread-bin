#include <TextEditor.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <fstream>
#include <sstream>

namespace breadbin::gui {
    void TextEditor::open_file(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::stringstream ss;
            ss << file.rdbuf();
            m_buffer = ss.str();
            m_current_path = path;
            m_is_open = true;
            m_dirty = false;
        }
    }

    void TextEditor::save_file() {
        std::ofstream file(m_current_path);
        if (file.is_open()) {
            file << m_buffer;
            m_dirty = false;
        }
    }

    void TextEditor::render(bool* p_open) {
        if (!m_is_open) {
            return;
        }

        ImGui::Begin("Raw Editor", p_open, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::MenuItem("Save", "Ctrl+S", false, m_dirty)) {
                save_file();
            }
            ImGui::EndMenuBar();
        }

        if (ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsKeyPressed(ImGuiKey_S)) {
            save_file();
        }

        ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;

        if (ImGui::InputTextMultiline("##source", &m_buffer, ImVec2(-FLT_MIN, -FLT_MIN), flags)) {
            m_dirty = true;
        }

        ImGui::End();
    }
} // namespace breadbin::gui
