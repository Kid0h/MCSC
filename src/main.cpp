#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <sstream>
#include <algorithm>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "imgui.h"

#include "api.h"

#define WINDOW_WIDTH 550
#define WINDOW_HEIGHT 54
// #define WINDOW_WIDTH 640
// #define WINDOW_HEIGHT 480

// GLFW error callback
static void glfw_error_callback(int error, const char *description) {
    return;
}

#ifdef WIN32
int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main(int argc, char* argv[]) {
#endif
    // GLFW Setup
    glfwSetErrorCallback(glfw_error_callback);
    glfwInit();

    // Window settings
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    /* OpenGL + GLSL versions */
    const char* glsl_version = "#version 440";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Create & use window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Minecraft Skin Cloner", NULL, NULL);
    if (!window) {
        return -1;
    } glfwMakeContextCurrent(window);

    // VSync
    glfwSwapInterval(1);

    // Load OpenGL
    gladLoadGL();

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = nullptr; io.LogFilename = nullptr;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Skin API
    bool empty = true;
    API api(&empty);
    std::string profile_buffer;
    profile_buffer.resize(36 * 50);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Viewport
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        
        // Clear
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Poll for event
        glfwPollEvents();

        // ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main window
        static bool main_window = true;
        {
            // ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowBgAlpha(0.85f);
            ImGui::SetNextWindowSize({WINDOW_WIDTH, WINDOW_HEIGHT});

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

            // ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | 

            ImGui::Begin("Minecraft Skin Cloner", &main_window,
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse
            ); if (!main_window) {
                glfwSetWindowShouldClose(window, true);
            }

            // Profile input
            bool triggered = false;
            ImGui::Text("%s", "Username/UUID:");
            ImGui::SameLine();
            bool text_triggered = ImGui::InputText("##profile_label", profile_buffer.data(), profile_buffer.capacity(), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::SameLine();
            
            bool _empty = empty;
            if (!_empty) {
                ImGui::BeginDisabled();
            }
            bool button_triggered = ImGui::Button("Clone", {ImGui::GetContentRegionAvail().x, 20});
            if (!_empty) {
                ImGui::EndDisabled();
            }
            
            if ((triggered = button_triggered || text_triggered) && strlen(profile_buffer.c_str()) > 0) {
                // Parse
                std::vector<std::string> profiles;
                std::stringstream ss(profile_buffer);

                while (ss.good()) {
                    std::string profile;
                    std::getline(ss, profile, ',');
                    profile.erase(std::remove_if(profile.begin(), profile.end(), ::isspace), profile.end()); // Remove spaces
                    profiles.push_back(profile);
                }

                // Queue profile/s
                for (const std::string& data : profiles) {
                    size_t len = strlen(data.c_str());
                    api.skin({data, (len == 32 || len == 36) ? API::Profile::Type::uuid : API::Profile::Type::username});
                }
            }
            
            auto x = ImGui::GetWindowSize();
            ImGui::End();
            ImGui::PopStyleVar(2);
        }

        // ImGui render
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        // Present
        glfwSwapBuffers(window);
    }

    // Cleanup
    /* ImGui */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    /* GLFW */
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}