/**
 * @File Application.cpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/15
 * @Brief This file is part of Nova.
 */

#include "./Application.hpp"

#include <imgui.h>
#include "./imgui_impl_opengl3.h"
#include "./imgui_impl_glfw.h"

#include "glad/include/glad/glad.h"

using namespace elma;

Application::Application(const AppConfig& config)
{
    LogInfo("Nova 开始运行...");

    _showUI  = config.showUI;
    _vsyncOn = config.windowDesc.enableVSync;

    /// 创建初始化各种部件
    if (!config.headless) {
        auto windowDesc = config.windowDesc;

        // Create the window
        _pWindow = Window::Create(windowDesc, this);
        _pWindow->setWindowIcon(std::filesystem::current_path() / "Data/Fairy-Tale-Castle-Princess.ico");
    }

    _initUI();
}

Application::~Application()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    /// 销毁各种部件
    _pWindow.reset();

    LogInfo("Nova 已停止运行");
}

int Application::run()
{
    _runInternal();
    return _returnCode;
}

void Application::resizeFrameBuffer(uint32_t width, uint32_t height)
{
    // 有窗口与无窗口
    if (_pWindow) {
        _pWindow->resize(width, height);
    }
    else {
        _resizeTargetFBO(width, height);
    }
}

void Application::renderFrame()
{
    /// 这里执行主要的渲染逻辑
    _renderUI();

    const auto sz = _pWindow->getClientAreaSize();
    glViewport(0, 0, sz.x, sz.y);

    glClearColor(0.45f, 0.55f, 0.60f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //        glRasterPos2i(-1, 1);
    //        glPixelZoom(1.0f, -1.0f);
    //        glDrawPixels(width, height,GL_RGBA,GL_UNSIGNED_BYTE, pixels);
}

void Application::shutdown(int returnCode)
{
    _shouldTerminate = true;
    _returnCode      = returnCode;
    if (_pWindow)
        _pWindow->shutdown();
}

AppConfig Application::getConfig() const
{
    AppConfig c;
    c.windowDesc = _pWindow->getDesc();
    c.showUI     = _showUI;
    return c;
}

std::string Application::getKeyboardShortcutsStr()
{
    constexpr char help[] = "ESC - Quit\n"
                            "V - Toggle VSync\n"
                            "MouseWheel - Change level of zoom\n";

    return help;
}

void Application::handleWindowSizeChange()
{
    /// 处理窗口尺寸改变逻辑
    ELMA_ASSERT(_pWindow);

    // Tell the device to resize the swap chain
    auto newSize    = _pWindow->getClientAreaSize();
    uint32_t width  = newSize.x;
    uint32_t height = newSize.y;
}

void Application::handleRenderFrame()
{
    renderFrame();
    _renderUI();

    _inputState.endFrame();
}

void Application::handleKeyboardEvent(const KeyboardEvent& keyEvent)
{
    _inputState.onKeyEvent(keyEvent);
    if (onKeyEvent(keyEvent))
        return;

    if (keyEvent.type == KeyboardEvent::Type::KeyPressed) {
        if (keyEvent.hasModifier(Modifier::Ctrl)) {
        }
        else if (keyEvent.mods == ModifierFlags::None) {
            switch (keyEvent.key) {
            case Key::Escape : _pWindow->shutdown(); break;
            default          : break;
            }
        }
    }
}

void Application::handleMouseEvent(const MouseEvent& mouseEvent)
{
    _inputState.onMouseEvent(mouseEvent);
    if (onMouseEvent(mouseEvent))
        return;
}

void Application::handleDroppedFile(const std::filesystem::path& path)
{
    onDroppedFile(path);
}

void Application::_resizeTargetFBO(uint32_t width, uint32_t height)
{
    onResize(width, height);
}

void Application::_initUI()
{
    // ImGui 初始化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(_pWindow->getHandle(), true);
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void Application::_saveConfigToFile()
{
}

void Application::_renderUI()
{
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    bool show_demo_window    = true;
    bool show_another_window = false;
    ImVec4 clear_color       = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f     = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                     // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);       // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button(
                "Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window) {
        ImGui::Begin(
            "Another Window",
            &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
    
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::_runInternal()
{
    if (_pWindow) {
        _pWindow->msgLoop();
    }
    else {
        while (!_shouldTerminate)
            handleRenderFrame();
    }

    onShutdown();
}
