#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <breadbin/LoafFile.hpp>
#include <cstdio>
#include <iostream>
#include <optional>
#include <nfd.hpp>

std::optional<std::filesystem::path> current_loaf_path;
bool loaf_dirty = false;

bool save_loaf_to_path(breadbin::core::LoafFile& loaf, const std::filesystem::path& path) {
    std::cout << "Attempting to save loaf to: " << path << std::endl;
    if (loaf.save_to_file(path)) {
        loaf_dirty = false;
        std::cout << "Save successful!" << std::endl;
        return true;
    }
    std::cerr << "Save failed! Cannot open file: " << path << std::endl;
    return false;
}


void save_loaf_as(breadbin::core::LoafFile& loaf, std::optional<std::filesystem::path>& path) {
    NFD::UniquePath outPath;
    nfdfilteritem_t filter[] = {{"Loaf files", "loaf"}};

    nfdresult_t result = NFD::SaveDialog(outPath, filter, 1);
    if (result == NFD_OKAY) {
        std::filesystem::path save_path = outPath.get();  // convert to proper path
        path = save_path;  // store in session
        std::cout << "Saving loaf to: " << save_path << std::endl;
        if (!save_loaf_to_path(loaf, save_path)) {
            std::cerr << "Failed to save loaf!" << std::endl;
        }
    } else if (result == NFD_CANCEL) {
        std::cout << "Save As cancelled by user." << std::endl;
    } else {
        std::cerr << "NFD error: " << NFD::GetError() << std::endl;
    }
}


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

    if (NFD::Init() != NFD_OKAY) {
        std::cerr << "Failed to initialize NFD." << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    breadbin::core::LoafFile current_loaf;
    current_loaf.name = "Untitled Loaf";

    static char name_buf[128] = {0};
    static bool initialized = false;

    while (! glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        std::string title = current_loaf.name.empty() ? "Untitled Loaf" : current_loaf.name;
        if (loaf_dirty) title += " *";
        glfwSetWindowTitle(window, title.c_str());

        if (! initialized) {
            std::snprintf(name_buf, sizeof(name_buf), "%s", current_loaf.name.c_str());
            initialized = true;
        }

        ImGui::Begin("Bread Bin Editor");

        if (ImGui::InputText("Loaf Name", name_buf, sizeof(name_buf))) {
            current_loaf.name = name_buf;
            if (!loaf_dirty) {
                loaf_dirty = true;
            }
        }

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
                    loaf_dirty = true;
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
                        loaf_dirty = true;
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

        ImGui::BeginDisabled(!loaf_dirty);
        if (ImGui::Button("Save")) {
            std::cout << "[DEBUG] Save button pressed" << std::endl;
            if (current_loaf_path.has_value()) {
                save_loaf_to_path(current_loaf, *current_loaf_path);
            } else {
                save_loaf_as(current_loaf, current_loaf_path);
            }
         }
        ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Save As")) {
            std::cout << "[DEBUG] Save As button pressed" << std::endl;
            save_loaf_as(current_loaf, current_loaf_path);
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

    NFD::Quit();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
