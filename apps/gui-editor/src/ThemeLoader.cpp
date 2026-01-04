#include <ThemeLoader.hpp>
#include <imgui.h>
#include <toml++/toml.h>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>

namespace breadbin::theme {

    static std::filesystem::path configDir;
    static std::filesystem::path themesDir;
    static std::filesystem::path configFile;
    static std::filesystem::path activeThemePath;

    static std::filesystem::path homeDir() {
        if (const char* h = std::getenv("HOME")) {
            return h;
        }
        return ".";
    }

    static void ensureDirs() {
        if (configDir.empty()) {
            configDir = homeDir() / ".config" / "the-bread-bin";
            themesDir = configDir / "themes";
            configFile = configDir / "config.toml";
        }

        std::filesystem::create_directories(themesDir);
    }

    void ApplyDefaultTheme() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        style.FrameRounding = 6.f;
        style.WindowRounding = 6.f;
        style.GrabRounding = 6.f;
        style.ScrollbarRounding = 6.f;

        style.WindowPadding = {10, 10};
        style.FramePadding = {6, 4};
        style.ItemSpacing = {6, 4};

        colors[ImGuiCol_Text] = {0.25f, 0.23f, 0.22f, 1.0f};
        colors[ImGuiCol_TextDisabled] = {0.42f, 0.40f, 0.38f, 1.0f};
        colors[ImGuiCol_WindowBg] = {0.98f, 0.97f, 0.96f, 1.0f};
        colors[ImGuiCol_ChildBg] = {0.96f, 0.95f, 0.94f, 1.0f};
        colors[ImGuiCol_PopupBg] = {0.96f, 0.95f, 0.94f, 1.0f};

        colors[ImGuiCol_FrameBg] = {0.94f, 0.93f, 0.92f, 1.0f};
        colors[ImGuiCol_FrameBgHovered] = {0.91f, 0.90f, 0.89f, 1.0f};
        colors[ImGuiCol_FrameBgActive] = {0.91f, 0.90f, 0.89f, 1.0f};

        colors[ImGuiCol_Button] = {0.94f, 0.93f, 0.92f, 1.0f};
        colors[ImGuiCol_ButtonHovered] = {0.91f, 0.90f, 0.89f, 1.0f};
        colors[ImGuiCol_ButtonActive] = {0.91f, 0.90f, 0.89f, 1.0f};

        colors[ImGuiCol_Header] = {0.94f, 0.93f, 0.92f, 1.0f};
        colors[ImGuiCol_HeaderHovered] = {0.91f, 0.90f, 0.89f, 1.0f};
        colors[ImGuiCol_HeaderActive] = {0.91f, 0.90f, 0.89f, 1.0f};

        colors[ImGuiCol_Separator] = {0.91f, 0.90f, 0.89f, 1.0f};
        colors[ImGuiCol_Tab] = {0.94f, 0.93f, 0.92f, 1.0f};
        colors[ImGuiCol_TabHovered] = {0.91f, 0.90f, 0.89f, 1.0f};
        colors[ImGuiCol_TabActive] = {0.91f, 0.90f, 0.89f, 1.0f};
        colors[ImGuiCol_TabUnfocused] = {0.96f, 0.95f, 0.94f, 1.0f};
        colors[ImGuiCol_TabUnfocusedActive] = {0.94f, 0.93f, 0.92f, 1.0f};

        colors[ImGuiCol_TextSelectedBg] = {0.26f, 0.59f, 0.98f, 0.35f};
        colors[ImGuiCol_NavHighlight] = {0.26f, 0.59f, 0.98f, 1.0f};
        colors[ImGuiCol_ModalWindowDimBg] = {0.8f, 0.8f, 0.8f, 0.35f};
    }

    bool LoadThemeFromFile(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            return false;
        }

        ApplyDefaultTheme();

        try {
            toml::table tbl = toml::parse_file(path.string());
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* colors = style.Colors;

            if (auto s = tbl["style"].as_table()) {
                auto getFloat = [&](const char* k, float d) {
                    return s->contains(k) ? float((*s)[k].value<double>().value_or(d)) : d;
                };

                auto getVec2 = [&](const char* k, ImVec2 d) {
                    auto a = (*s)[k].as_array();
                    if (!a || a->size() < 2) {
                        return d;
                    }
                    return ImVec2(float((*a)[0].value<double>().value_or(d.x)), float((*a)[1].value<double>().value_or(d.y)));
                };

                style.FrameRounding = getFloat("FrameRounding", style.FrameRounding);
                style.WindowRounding = getFloat("WindowRounding", style.WindowRounding);
                style.GrabRounding = getFloat("GrabRounding", style.GrabRounding);
                style.ScrollbarRounding = getFloat("ScrollbarRounding", style.ScrollbarRounding);

                style.WindowPadding = getVec2("WindowPadding", style.WindowPadding);
                style.FramePadding = getVec2("FramePadding", style.FramePadding);
                style.ItemSpacing = getVec2("ItemSpacing", style.ItemSpacing);
            }

            if (auto c = tbl["colors"].as_table()) {
                std::unordered_map<std::string, ImGuiCol> map;
                for (int i = 0; i < ImGuiCol_COUNT; ++i) {
                    map[ImGui::GetStyleColorName(i)] = (ImGuiCol)i;
                }

                for (auto& [k, v] : *c) {
                    auto arr = v.as_array();
                    if (!arr || arr->size() < 4) {
                        continue;
                    }

                    auto it = map.find(std::string(k.str()));
                    if (it == map.end()) {
                        continue;
                    }

                    colors[it->second] = ImVec4(float((*arr)[0].value<double>().value_or(0.f)), float((*arr)[1].value<double>().value_or(0.f)), float((*arr)[2].value<double>().value_or(0.f)), float((*arr)[3].value<double>().value_or(1.f)));
                }
            }

            activeThemePath = path;
            return true;

        } catch (const toml::parse_error& e) {
            std::cerr << "Theme parse error: " << e.description() << "\n";
            return false;
        }
    }

    std::vector<std::filesystem::path> DiscoverThemes() {
        ensureDirs();

        std::vector<std::filesystem::path> out;
        for (auto& e : std::filesystem::directory_iterator(themesDir)) {
            if (e.path().extension() == ".toml") {
                out.push_back(e.path());
            }
        }

        std::sort(out.begin(), out.end());
        return out;
    }

    void SaveActiveTheme() {
        ensureDirs();

        toml::table cfg;

        toml::table ui;
        ui.insert(
            "active_theme",
            activeThemePath.empty() ? "" : std::filesystem::relative(activeThemePath, configDir).string()
        );

        cfg.insert("ui", std::move(ui));

        std::ofstream out(configFile);
        out << cfg;
    }

    void LoadPersistedTheme() {
        ensureDirs();

        if (!std::filesystem::exists(configFile)) {
            ApplyDefaultTheme();
            return;
        }

        try {
            toml::table cfg = toml::parse_file(configFile.string());
            auto path = cfg["ui"]["active_theme"].value<std::string>();

            if (!path || path->empty()) {
                ApplyDefaultTheme();
                return;
            }

            std::filesystem::path full = configDir / *path;
            if (!LoadThemeFromFile(full)) {
                ApplyDefaultTheme();
            }

        } catch (...) {
            ApplyDefaultTheme();
        }
    }

    const std::filesystem::path& GetActiveTheme() {
        return activeThemePath;
    }

} // namespace breadbin::theme
