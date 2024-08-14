/**
 * @File Application.cpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/15
 * @Brief This file is part of Nova.
 */

#include "./Application.hpp"

#include "Parsers/ParseScene.hpp"
#include "Parallel.hpp"
#include "Image.hpp"
#include "Render.hpp"
#include <embree4/rtcore.h>
#include <memory>
#include "Timer.hpp"

#include <imgui.h>
#include "./imgui_impl_opengl3.h"
#include "./imgui_impl_glfw.h"

#include "glad/include/glad/glad.h"

using namespace elma;

namespace {

RTCDevice embreeDevice;
std::unique_ptr<Scene> scene = nullptr;
std::unique_ptr<Timer> timer = nullptr;

struct RenderRecords
{
    Image3 acc;
    double accCount;
    Image3f display;

    std::string sceneName;
};

RenderRecords renderRec;

} // namespace

Application::Application(const AppConfig& config)
{
    LogInfo("Nova 开始运行...");

    /// 创建初始化各种部件

    embreeDevice = rtcNewDevice(nullptr);
    ParallelInit(config.numThreads);

    timer = std::make_unique<Timer>();
    {
        tick(*timer);
        LogInfo("解析并构造场景 '{}'...", config.inputSceneFilename);
        scene = ParseScene(config.inputSceneFilename, embreeDevice);
        LogInfo("场景构造完成，花费 '{}' 秒", tick(*timer));
    }

    scene->options.samplesPerPixel = 1;

    renderRec.sceneName = std::filesystem::path(config.inputSceneFilename).stem().string();

    auto windowDesc   = config.windowDesc;
    windowDesc.width  = scene->camera.width;
    windowDesc.height = scene->camera.height;

    // Create the window
    _pWindow = Window::Create(windowDesc, this);
    _pWindow->setWindowIcon(std::filesystem::current_path() / "Data/Fairy-Tale-Castle-Princess.ico");

    renderRec.acc      = Image3{scene->camera.width, scene->camera.height};
    renderRec.accCount = 0;
    renderRec.display  = Image3f{scene->camera.width, scene->camera.height};

    _initUI();
}

Application::~Application()
{
    ParallelCleanup();
    rtcReleaseDevice(embreeDevice);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

//    {
//        const auto w = scene->camera.width;
//        const auto h = scene->camera.height;
//        Image3 img{w, h};
//        
//        const auto f = Real(1.0) / (std::max(0.001, renderRec.accCount));
//        for (int y = 0; y < h; ++y) {
//            for (int x = 0; x < w; ++x) {
//                const auto acc = renderRec.acc(x, y);
//                
//                img(x, y).x = (Real)(std::clamp(acc.x * f, 0., 1.));
//                img(x, y).y = (Real)(std::clamp(acc.y * f, 0., 1.));
//                img(x, y).z = (Real)(std::clamp(acc.z * f, 0., 1.));
//            }
//        }
//        
//        ImageWrite("output.exr", img);
//    }
    
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

    const auto w = scene->camera.width;
    const auto h = scene->camera.height;

    glViewport(0, 0, w, h);
    const auto img = Render(*scene);
    

    renderRec.accCount             += 1;
    scene->options.accumulateCount += 1;
//    
//    if (renderRec.accCount == 1)
//    {
//        ImageWrite("output.exr", img);
//    }

    Real f = Real(1.0) / (std::max(0.001, renderRec.accCount));
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const auto acc      = renderRec.acc(x, y) + img(x, y);
            renderRec.acc(x, y) = acc;

            renderRec.display(x, y).x = std::pow((float)(std::clamp(acc.x * f, 0., 1.)), 1.0f / 2.2);
            renderRec.display(x, y).y = std::pow((float)(std::clamp(acc.y * f, 0., 1.)), 1.0f / 2.2);
            renderRec.display(x, y).z = std::pow((float)(std::clamp(acc.z * f, 0., 1.)), 1.0f / 2.2);
        }
    }

    glRasterPos2i(-1, 1);
    glPixelZoom(1.0f, -1.0f);
    glDrawPixels(w, h, GL_RGB, GL_FLOAT, renderRec.display.data.data());
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

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::SetNextWindowBgAlpha(0.23f);
        ImGui::Begin("Render Stats");
        ImGui::Text("Scene: %s", renderRec.sceneName.c_str());
        ImGui::Text("Acc Count = %lld", static_cast<uint64_t>(renderRec.accCount));

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
