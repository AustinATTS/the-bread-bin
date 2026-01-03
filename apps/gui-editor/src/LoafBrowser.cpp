#include <LoafBrowser.hpp>

#include <imgui.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace breadbin::gui {

LoafBrowser::LoafBrowser(bool& dirty_flag, core::LoafFile& active_loaf)
    : m_dirty(dirty_flag)
    , m_active_loaf(active_loaf)
{
    const char* home = std::getenv("HOME");
    if (home) {
        m_root_path = std::filesystem::path(home) / ".config" / "the-bread-bin";
    }
    refresh_files();
}

void LoafBrowser::refresh_files()
{
    m_files.clear();

    if (!std::filesystem::exists(m_root_path)) {
        return;
    }

    try {
        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(m_root_path)) {
            if (entry.is_regular_file()) {
                m_files.push_back(entry.path());
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error browsing files: " << e.what() << std::endl;
    }
}

void LoafBrowser::render(bool* p_open)
{
    if (!p_open || !*p_open) {
        return;
    }

    if (!ImGui::Begin("Loaf Browser", p_open)) {
        ImGui::End();
        return;
    }

    int index = 0;
    for (const auto& path : m_files) {
        // Stable, non-string ID — no collisions, no temporaries
        ImGui::PushID(index);

        const std::string label =
            path.lexically_relative(m_root_path).string();

        if (ImGui::Selectable(
                label.c_str(),
                false,
                ImGuiSelectableFlags_AllowDoubleClick)) {

            if (ImGui::IsMouseDoubleClicked(0)) {
                m_selected_path = path;
                handle_file_action(path);
            }
        }

        // Context menu bound to the last item — no explicit ID needed
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Edit / Load")) {
                m_selected_path = path;
                handle_file_action(path);
            }

            if (ImGui::MenuItem("Delete")) {
                m_selected_path = path;
                m_show_delete_modal = true;
            }

            ImGui::EndPopup();
        }

        ImGui::PopID();
        ++index;
    }

    // ---------- Unsaved changes modal ----------
    if (m_show_unsaved_modal) {
        ImGui::OpenPopup("Unsaved Changes?");
    }

    if (ImGui::BeginPopupModal(
            "Unsaved Changes?",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::TextUnformatted(
            "You have unsaved changes in your current loaf.\n"
            "What would you like to do?");

        if (ImGui::Button("Save and Load")) {
            m_active_loaf.save_to_file("previous_auto.loaf");
            m_active_loaf.load_from_file(m_selected_path);
            m_dirty = false;
            m_show_unsaved_modal = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Just Load")) {
            m_active_loaf.load_from_file(m_selected_path);
            m_dirty = false;
            m_show_unsaved_modal = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel")) {
            m_show_unsaved_modal = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // ---------- Delete confirmation modal ----------
    if (m_show_delete_modal) {
        ImGui::OpenPopup("Confirm Delete?");
    }

    if (ImGui::BeginPopupModal(
            "Confirm Delete?",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::Text(
            "Are you sure you want to delete:\n%s ?",
            m_selected_path.filename().string().c_str());

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

void LoafBrowser::handle_file_action(const std::filesystem::path& path)
{
    if (path.extension() == ".toml") {
        std::cout << "Opening theme editor for: " << path << std::endl;
        return;
    }

    if (path.extension() == ".loaf") {
        if (m_dirty) {
            m_show_unsaved_modal = true;
        }
        else {
            m_active_loaf.load_from_file(path);
        }
    }
}

} // namespace breadbin::gui
