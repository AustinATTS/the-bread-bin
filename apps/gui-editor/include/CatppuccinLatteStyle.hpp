#pragma once
#include <imgui.h>

inline void ApplyCatppuccinLatteTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 6.0f;
    style.WindowRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(6, 4);

    ImVec4 latte_base = ImVec4(0.98f, 0.97f, 0.96f, 1.00f);
    ImVec4 latte_surface0 = ImVec4(0.96f, 0.95f, 0.94f, 1.00f);
    ImVec4 latte_surface1 = ImVec4(0.94f, 0.93f, 0.92f, 1.00f);
    ImVec4 latte_surface2 = ImVec4(0.91f, 0.90f, 0.89f, 1.00f);
    ImVec4 latte_overlay0 = ImVec4(0.85f, 0.84f, 0.83f, 1.00f);
    ImVec4 latte_overlay1 = ImVec4(0.78f, 0.77f, 0.76f, 1.00f);
    ImVec4 latte_text = ImVec4(0.25f, 0.23f, 0.22f, 1.00f);
    ImVec4 latte_subtext0 = ImVec4(0.42f, 0.40f, 0.38f, 1.00f);
    ImVec4 latte_blue = ImVec4(0.20f, 0.42f, 0.80f, 1.00f);
    ImVec4 latte_green = ImVec4(0.20f, 0.60f, 0.30f, 1.00f);
    ImVec4 latte_red = ImVec4(0.81f, 0.20f, 0.22f, 1.00f);
    ImVec4 latte_yellow = ImVec4(0.75f, 0.58f, 0.19f, 1.00f);
    ImVec4 latte_peach = ImVec4(0.82f, 0.38f, 0.29f, 1.00f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = latte_text;
    colors[ImGuiCol_TextDisabled] = latte_subtext0;
    colors[ImGuiCol_WindowBg] = latte_base;
    colors[ImGuiCol_ChildBg] = latte_surface0;
    colors[ImGuiCol_PopupBg] = latte_surface0;
    colors[ImGuiCol_Border] = latte_overlay0;
    colors[ImGuiCol_BorderShadow] = latte_overlay1;
    colors[ImGuiCol_FrameBg] = latte_surface1;
    colors[ImGuiCol_FrameBgHovered] = latte_surface2;
    colors[ImGuiCol_FrameBgActive] = latte_surface2;
    colors[ImGuiCol_TitleBg] = latte_surface1;
    colors[ImGuiCol_TitleBgActive] = latte_surface2;
    colors[ImGuiCol_TitleBgCollapsed] = latte_surface0;
    colors[ImGuiCol_MenuBarBg] = latte_surface1;
    colors[ImGuiCol_ScrollbarBg] = latte_surface0;
    colors[ImGuiCol_ScrollbarGrab] = latte_overlay0;
    colors[ImGuiCol_ScrollbarGrabHovered] = latte_overlay1;
    colors[ImGuiCol_ScrollbarGrabActive] = latte_overlay1;
    colors[ImGuiCol_CheckMark] = latte_blue;
    colors[ImGuiCol_SliderGrab] = latte_blue;
    colors[ImGuiCol_SliderGrabActive] = latte_blue;
    colors[ImGuiCol_Button] = latte_surface1;
    colors[ImGuiCol_ButtonHovered] = latte_surface2;
    colors[ImGuiCol_ButtonActive] = latte_surface2;
    colors[ImGuiCol_Header] = latte_surface1;
    colors[ImGuiCol_HeaderHovered] = latte_surface2;
    colors[ImGuiCol_HeaderActive] = latte_surface2;
    colors[ImGuiCol_ResizeGrip] = latte_overlay0;
    colors[ImGuiCol_ResizeGripHovered] = latte_overlay1;
    colors[ImGuiCol_ResizeGripActive] = latte_overlay1;
    colors[ImGuiCol_PlotLines] = latte_blue;
    colors[ImGuiCol_PlotLinesHovered] = latte_peach;
    colors[ImGuiCol_PlotHistogram] = latte_yellow;
    colors[ImGuiCol_PlotHistogramHovered] = latte_red;
    colors[ImGuiCol_TextSelectedBg] = latte_overlay1;
    colors[ImGuiCol_Separator] = latte_overlay0;
    colors[ImGuiCol_SeparatorHovered] = latte_overlay1;
    colors[ImGuiCol_SeparatorActive] = latte_overlay1;
}
