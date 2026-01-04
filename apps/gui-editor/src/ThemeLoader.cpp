#include <ThemeLoader.hpp>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <toml++/toml.h>
#include <imgui.h>

namespace breadbin::theme {

    static std::filesystem::path configPath;

    void SetConfigPath(const std::string& path) {
        configPath = path;
    }

    void ApplyTheme() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        struct DefaultTheme {
            float FrameRounding = 6.f;
            float WindowRounding = 6.f;
            float GrabRounding = 6.f;
            float ScrollbarRounding = 6.f;
            ImVec2 WindowPadding = {10, 10};
            ImVec2 FramePadding = {6, 4};
            ImVec2 ItemSpacing = {6, 4};

            std::unordered_map<ImGuiCol, ImVec4> Colors {
                {ImGuiCol_Text, {0.25f, 0.23f, 0.22f, 1.0f}},
                {ImGuiCol_TextDisabled, {0.42f, 0.40f, 0.38f, 1.0f}},
                {ImGuiCol_WindowBg, {0.98f, 0.97f, 0.96f, 1.0f}},
                {ImGuiCol_ChildBg, {0.96f, 0.95f, 0.94f, 1.0f}},
                {ImGuiCol_PopupBg, {0.96f, 0.95f, 0.94f, 1.0f}},

                {ImGuiCol_Border, {0.0f, 0.0f, 0.0f, 0.0f}},
                {ImGuiCol_FrameBg, {0.94f, 0.93f, 0.92f, 1.0f}},
                {ImGuiCol_FrameBgHovered, {0.91f, 0.90f, 0.89f, 1.0f}},
                {ImGuiCol_FrameBgActive, {0.91f, 0.90f, 0.89f, 1.0f}},

                {ImGuiCol_Button, {0.94f, 0.93f, 0.92f, 1.0f}},
                {ImGuiCol_ButtonHovered, {0.91f, 0.90f, 0.89f, 1.0f}},
                {ImGuiCol_ButtonActive, {0.91f, 0.90f, 0.89f, 1.0f}},

                {ImGuiCol_Header, {0.94f, 0.93f, 0.92f, 1.0f}},
                {ImGuiCol_HeaderHovered, {0.91f, 0.90f, 0.89f, 1.0f}},
                {ImGuiCol_HeaderActive, {0.91f, 0.90f, 0.89f, 1.0f}},

                {ImGuiCol_Separator, {0.91f, 0.90f, 0.89f, 1.0f}},
                {ImGuiCol_SeparatorHovered, {0.91f, 0.90f, 0.89f, 1.0f}},
                {ImGuiCol_SeparatorActive, {0.91f, 0.90f, 0.89f, 1.0f}},

                {ImGuiCol_ResizeGrip, {0.94f, 0.93f, 0.92f, 1.0f}},
                {ImGuiCol_ResizeGripHovered, {0.91f, 0.90f, 0.89f, 1.0f}},
                {ImGuiCol_ResizeGripActive, {0.91f, 0.90f, 0.89f, 1.0f}},

                {ImGuiCol_Tab, {0.94f, 0.93f, 0.92f, 1.0f}},
                {ImGuiCol_TabHovered, {0.91f, 0.90f, 0.89f, 1.0f}},
                {ImGuiCol_TabActive, {0.91f, 0.90f, 0.89f, 1.0f}},
                {ImGuiCol_TabUnfocused, {0.96f, 0.95f, 0.94f, 1.0f}},
                {ImGuiCol_TabUnfocusedActive, {0.94f, 0.93f, 0.92f, 1.0f}},

                {ImGuiCol_PlotLines, {0.25f,0.8f,0.5f,1.0f}},
                {ImGuiCol_PlotLinesHovered, {0.3f,0.9f,0.6f,1.0f}},
                {ImGuiCol_PlotHistogram, {0.25f,0.7f,0.4f,1.0f}},
                {ImGuiCol_PlotHistogramHovered, {0.3f,0.8f,0.5f,1.0f}},

                {ImGuiCol_TextSelectedBg, {0.26f,0.59f,0.98f,0.35f}},
                {ImGuiCol_DragDropTarget, {1.0f,1.0f,0.0f,0.9f}},
                {ImGuiCol_NavHighlight, {0.26f,0.59f,0.98f,1.0f}},
                {ImGuiCol_NavWindowingHighlight, {1.0f,1.0f,0.0f,0.7f}},
                {ImGuiCol_NavWindowingDimBg, {0.8f,0.8f,0.8f,0.2f}},
                {ImGuiCol_ModalWindowDimBg, {0.8f,0.8f,0.8f,0.35f}}
            };
        } defaultTheme;

        style.FrameRounding = defaultTheme.FrameRounding;
        style.WindowRounding = defaultTheme.WindowRounding;
        style.GrabRounding = defaultTheme.GrabRounding;
        style.ScrollbarRounding = defaultTheme.ScrollbarRounding;
        style.WindowPadding = defaultTheme.WindowPadding;
        style.FramePadding = defaultTheme.FramePadding;
        style.ItemSpacing = defaultTheme.ItemSpacing;

        for (const auto& [idx, col] : defaultTheme.Colors) {
            colors[idx] = col;
        }

        colors[ImGuiCol_FrameBg] = defaultTheme.Colors[ImGuiCol_FrameBg];
        colors[ImGuiCol_FrameBgHovered] = defaultTheme.Colors[ImGuiCol_FrameBgHovered];
        colors[ImGuiCol_FrameBgActive] = defaultTheme.Colors[ImGuiCol_FrameBgActive];
        colors[ImGuiCol_PopupBg] = defaultTheme.Colors[ImGuiCol_PopupBg];

        if (configPath.empty()) {
            configPath = std::filesystem::path(getenv("HOME")) / ".config" / "the-bread-bin" / "theme.toml";
        }

        if (!std::filesystem::exists(configPath)) {
            return;
        }

        try {
            toml::table config = toml::parse_file(configPath.string());

            if (auto s = config["style"].as_table()) {
                auto getFloat = [&](const std::string& key, float def) -> float {
                    if (s->contains(key)) {
                        return float((*s)[key].value<double>().value_or(def));
                    }
                    return def;
                };
                auto getVec2 = [&](const std::string& key, const ImVec2& def) -> ImVec2 {
                    if (!s->contains(key)) {
                        return def;
                    }
                    auto arr = (*s)[key].as_array();
                    if (!arr || arr->size() < 2) {
                        return def;
                    }
                    return ImVec2(float((*arr)[0].value<double>().value_or(def.x)), float((*arr)[1].value<double>().value_or(def.y)));
                };

                style.FrameRounding = getFloat("FrameRounding", style.FrameRounding);
                style.WindowRounding = getFloat("WindowRounding", style.WindowRounding);
                style.GrabRounding = getFloat("GrabRounding", style.GrabRounding);
                style.ScrollbarRounding = getFloat("ScrollbarRounding", style.ScrollbarRounding);
                style.WindowPadding = getVec2("WindowPadding", style.WindowPadding);
                style.FramePadding = getVec2("FramePadding", style.FramePadding);
                style.ItemSpacing = getVec2("ItemSpacing", style.ItemSpacing);
            }

            if (auto c = config["colors"].as_table()) {
                std::unordered_map<std::string, ImGuiCol> nameMap;
                for (int i = 0; i < ImGuiCol_COUNT; i++) {
                    nameMap[ImGui::GetStyleColorName(i)] = i;
                }

                for (auto& [key, val] : *c) {
                    std::string name{key.str()};
                    if (!val.is_array() || val.as_array()->size() < 4) {
                        continue;
                    }

                    ImGuiCol idx = ImGuiCol_Text;
                    if (nameMap.find(name) != nameMap.end()) {
                        idx = nameMap[name];
                    }

                    auto arr = val.as_array();
                    colors[idx] = ImVec4(float((*arr)[0].value<double>().value_or(colors[idx].x)), float((*arr)[1].value<double>().value_or(colors[idx].y)), float((*arr)[2].value<double>().value_or(colors[idx].z)), float((*arr)[3].value<double>().value_or(colors[idx].w)));
                }
            }

        } catch (const toml::parse_error& err) {
            std::cerr << "Failed to parse theme config: " << err.description() << "\n";
        }
    }
} // namespace breadbin::theme
