#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <breadbin/LoafFile.hpp>
#include <cstdio>
#include <iostream>

int main ( ) {
    if (! glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow * window = glfwCreateWindow(1280, 720, "The Bread Bin - Editor", nullptr, nullptr);
    if (! window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (! gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    breadbin::core::LoafFile current_loaf;
    current_loaf.app_name = "The Bread Bin - Editor";

    static char name_buf[128] = {0};
    static bool initialized = false;

    while (! glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (! initialized) {
            std::snprintf(name_buf, sizeof(name_buf), "%s", current_loaf.app_name.c_str());
            initialized = true;
        }

        ImGui::Begin("Bread Bin Editor");

        if (ImGui::Button("+ Add Action")) {
            current_loaf.actions.push_back({
            breadbin::core::ActionType::App,
            "",
            {}
            });
        }

        for (size_t i = 0; i < current_loaf.actions.size(); i++) {
            auto& action = current_loaf.actions[i];

            ImGui::PushID(static_cast<int>(i));

            const char* type_labels[] = {"App", "File", "Link"};
            int type_index = static_cast<int>(action.type);

            std::string header = type_labels[type_index] + std::string(": ") + (action.target.empty() ? "<unset>" : action.target);

            if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

                if (ImGui::Combo("Type", &type_index, type_labels, IM_ARRAYSIZE(type_labels))) {
                    action.type = static_cast<breadbin::core::ActionType>(type_index);
                }

                static char target_buf[256];
                std::snprintf(target_buf, sizeof(target_buf), "%s", action.target.c_str());

                if (ImGui::InputText("Target", target_buf, sizeof(target_buf))) {
                    action.target = target_buf;
                }

                ImGui::Separator();
                ImGui::Text("Arguments");

                if (ImGui::Button("+ Add Arguments")) {
                    action.args.push_back("");
                }

                for (size_t a = 0; a < action.args.size(); ++a) {
                    ImGui::PushID(static_cast<int>(a));

                    char arg_buf[128];
                    std::snprintf(arg_buf, sizeof(arg_buf), "%s", action.args[a].c_str());

                    if (ImGui::InputText("##arg", arg_buf, sizeof(arg_buf))) {
                        action.args[a] = arg_buf;
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Remove")) {
                        action.args.erase(action.args.begin() + a);
                        ImGui::PopID();
                        break;
                    }

                    ImGui::PopID();
                }

                ImGui::Separator();
                if (ImGui::Button("Remove Action")) {
                    current_loaf.actions.erase(current_loaf.actions.begin() + i);
                    ImGui::PopID();
                    break;
                }
            }

            ImGui::PopID();
        }

        if (ImGui::Button("Save Loaf File")) {
            std::filesystem::path save_path = "settings.loaf";
            if (current_loaf.save_to_file(save_path)) {
                std::cout << "Successfully saved config.loaf" << std::endl;
            } else {
                std::cout << "Failed to save config.loaf" << std::endl;
            }
        }

        ImGui::End();

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, & display_w, & display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
