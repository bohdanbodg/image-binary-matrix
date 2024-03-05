#include "gui.hpp"

#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

Application::Application(const char *windowName)
    : windowName(windowName), windowWidth(0), windowHeight(0), inited(false) {
}

Application::~Application() {
    this->dispose();
}

bool Application::init() {
    if (this->inited) {
        return false;
    }

    this->inited = true;

    this->clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    return this->initGLFW() && this->initImGui();
}

void Application::run() {
    if (!this->inited) {
        return;
    }

    // Main loop
    while (!glfwWindowShouldClose(this->window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        this->draw();

        ImGui::Render();

        const auto &clear_color = this->clearColor;
        glfwGetFramebufferSize(
            this->window,
            &this->windowWidth,
            &this->windowHeight
        );
        glViewport(0, 0, this->windowWidth, this->windowHeight);
        glClearColor(
            clear_color.x * clear_color.w,
            clear_color.y * clear_color.w,
            clear_color.z * clear_color.w,
            clear_color.w
        );
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(this->window);
    }
}

void Application::terminate() {
    glfwSetWindowShouldClose(this->window, GLFW_TRUE);
}

void Application::draw() {
    // Does nothing
}

void Application::dispose() {
    if (!this->inited) {
        return;
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(this->window);
    glfwTerminate();
}

bool Application::initGLFW() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return false;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    this->glslVersion = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    this->glslVersion = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
    // GL 3.0 + GLSL 130
    this->glslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create window with graphics context
    this->window
        = glfwCreateWindow(800, 600, this->windowName, nullptr, nullptr);
    if (this->window == nullptr) {
        return false;
    }

    glfwMakeContextCurrent(this->window);
    glfwSwapInterval(1); // Enable vsync

    return true;
}

bool Application::initImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags
        |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags
        |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    return ImGui_ImplGlfw_InitForOpenGL(this->window, true)
           && ImGui_ImplOpenGL3_Init(this->glslVersion);
}
