#include <LoafBrowser.hpp>
#include <imgui.h>

namespace breadbin::gui {
    LoafBrowser::LoafBrowser() {
        const char* home = std::getenv("HOME");
        config_path_ = std::filesystem::path(home) / ".config" / "the-bread-bin" / "loafs";
        refresh_loafs();
    }

    void LoafBrowser::refresh_loafs() {
        loaf_files_.clear();
        if (std::filesystem::exists(config_path_)) {
            for (const auto& entry : std::filesystem::directory_iterator(config_path_)) {
                if (entry.path().extension() == ".loaf") {
                    loaf_files_.push_back(entry.path());
                }
            }
        }
    }

    void LoafBrowser::render() {
        ImGui::Begin("Loaf Library");

        if (ImGui::Button("Refresh")) {
            refresh_loafs();
        }

        ImGui::Separator();

        for (const auto& path : loaf_files_) {
            if (ImGui::Selectable(path.filename().string().c_str())) {

            }
        }

        ImGui::End();
    }
} // namespace breadbin::gui
