#include <breadbin/LoafRunner.hpp>
#include <ThemeLoader.hpp>
#include <LoafBrowser.hpp>
#include <TextEditor.hpp>
#include <imgui.h>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>

namespace breadbin::gui {
    LoafBrowser::LoafBrowser(bool& dirty_flag, core::LoafFile& active_loaf, TextEditor& editor, std::optional<std::filesystem::path>& active_path) : m_dirty(dirty_flag), m_active_loaf(active_loaf), m_editor(editor), m_active_path(active_path) {
        const char* home = std::getenv("HOME");
        if (home) {
            m_root_path = std::filesystem::path(home) / ".config" / "the-bread-bin";
        }
        refresh_files();
    }

    void LoafBrowser::refresh_files() {
        m_files.clear();

        if (!std::filesystem::exists(m_root_path)) {
            return;
        }

        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(m_root_path)) {
                if (entry.is_regular_file()) {
                    auto ext = entry.path().extension();
                    if (ext == ".loaf" || ext == ".toml") {
                        m_files.push_back(entry.path());
                    }
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error browsing files: " << e.what() << std::endl;
        }
    }

    void LoafBrowser::render(bool* p_open) {
        if (!p_open || !*p_open) {
            return;
        }

        if (!ImGui::Begin("Loaf Browser", p_open)) {
            ImGui::End();
            return;
        }

        for (size_t i = 0; i < m_files.size(); ++i) {
            const auto& path = m_files[i];

            ImGui::PushID(static_cast<int>(i));

            std::string label = path.filename().string();
            try {
                label = path.lexically_relative(m_root_path).string();
            }
            catch (...) {

            }

            if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    m_selected_path = path;
                    handle_file_action(path);
                }
            }

            if (ImGui::BeginPopupContextItem()) {
                m_selected_path = path;
                if (path.extension() == ".loaf") {
                    if (ImGui::MenuItem("Run Loaf")) {
                        breadbin::core::LoafFile temploaf;
                        if (temploaf.load_from_file(path)) {
                            breadbin::core::LoafRunner::execute(temploaf);
                        }
                    }
                    ImGui::Separator();
                }

                if (ImGui::MenuItem("Load")) {
                    m_selected_path = path;
                    handle_file_action(path);
                }

                if (ImGui::MenuItem("Text Editor")) {
                    m_editor.open_file(path);
                }

                if (ImGui::MenuItem("Delete")) {
                    m_selected_path = path;
                    m_show_delete_modal = true;
                }

                ImGui::EndPopup();
            }
            ImGui::PopID();
        }

        if (m_show_unsaved_modal) {
            ImGui::OpenPopup("Unsaved Changes?");
        }
        if (ImGui::BeginPopupModal("Unsaved Changes?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("You have unsaved changes in your current loaf.\nWhat would you like to do?");
            ImGui::Separator();

            if (ImGui::Button("Save and Load", ImVec2(120, 0))) {
                m_active_loaf.save_to_file("previous_auto.loaf");
                if (m_active_loaf.load_from_file(m_selected_path)) {
                    m_active_path = m_selected_path;
                    m_dirty = false;
                }
                m_show_unsaved_modal = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Discard & Load", ImVec2(120, 0))) {
                if (m_active_loaf.load_from_file(m_selected_path)) {
                    m_active_path = m_selected_path;
                    m_dirty = false;
                }
                m_show_unsaved_modal = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                m_show_unsaved_modal = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (m_show_delete_modal) {
            ImGui::OpenPopup("Confirm Delete?");
        }

        if (ImGui::BeginPopupModal("Confirm Delete?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

            ImGui::Text("Are you sure you want to delete:\n%s?", m_selected_path.filename().string().c_str());

            if (ImGui::Button("Yes, Delete")) {
                std::filesystem::remove(m_selected_path);
                refresh_files();
                m_show_delete_modal = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("No")) {
                m_show_delete_modal = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void LoafBrowser::handle_file_action(const std::filesystem::path& path) {
        if (path.extension() == ".toml") {
            if (breadbin::theme::LoadThemeFromFile(path)) {
                breadbin::theme::SaveActiveTheme();
            }
        }

        if (path.extension() == ".loaf") {
            if (m_dirty) {
                m_selected_path = path;
                m_show_unsaved_modal = true;
            }
            else {
                if (m_active_loaf.load_from_file(path)) {
                    m_active_path = path;
                    m_dirty = false;
                }
            }
        }
    }

    void LoafBrowser::reload() {
        refresh_files();
    }
} // namespace breadbin::gui
