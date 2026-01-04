#include <TextEditor.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace breadbin::gui {
    void TextEditor::open_file(const std::filesystem::path& path) {
        if (path.empty()) {
            return;
        }

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[TextEditor] Failed to open: " << path << "\n";
            return;
        }

        std::stringstream ss;
        ss << file.rdbuf();

        m_buffer = ss.str();
        m_current_path = path;
        m_is_open = true;
        m_dirty = false;
    }

    void TextEditor::save_file() {
        if (m_current_path.empty()) {
            return;
        }

        std::ofstream file(m_current_path);
        if (!file.is_open()) {
            std::cerr << "[TextEditor] Failed to save: " << m_current_path << "\n";
            return;
        }

        file << m_buffer;
        file.close();

        m_dirty = false;
    }

    void TextEditor::render(bool* p_open) {
        if (!m_is_open) {
            return;
        }

        std::string window_name = "Text Editor: " + m_current_path.filename().string();
        if (m_dirty) window_name += "*";

        ImGui::Begin(window_name.c_str(), p_open, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save", "Ctrl+S", false, m_dirty)) {
                    save_file();
                }
                ImGui::EndMenu();
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

        if (!*p_open) {
            m_is_open = false;
        }

        ImGui::End();
    }

} // namespace breadbin::gui