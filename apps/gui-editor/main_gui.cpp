#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <breadbin/LoafFile.hpp>
#include <cstdio>
#include <iostream>
#include <stdio.h>

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

        if (ImGui::InputText("Application Name", name_buf, sizeof(name_buf))) {
            current_loaf.app_name = name_buf;
        }

        if (ImGui::Button("+ Add Arg")) {
            current_loaf.args.push_back("new_argument");
        }

        for (size_t i = 0; i < current_loaf.args.size(); i++) {
            char arg_buf[128];
            std::snprintf(arg_buf, sizeof(arg_buf), "%s", current_loaf.args[i].c_str());
            if (ImGui::InputText(("##arg" + std::to_string(i)).c_str(), arg_buf, sizeof(arg_buf))) {
                current_loaf.args[i] = arg_buf;
            }
            ImGui::SameLine();
            if (ImGui::Button(("Remove##" + std::to_string(i)).c_str())) {
                current_loaf.args.erase(current_loaf.args.begin() + i);
            }
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
