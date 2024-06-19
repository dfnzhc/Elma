/**
 * @File Application.cpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/15
 * @Brief This file is part of Nova.
 */

#include "./Application.hpp"
#include <glad/glad.h>

using namespace elma;

Application::Application(const AppConfig& config)
{
    _showUI  = config.showUI;

    /// 创建初始化各种部件
    auto windowDesc = config.windowDesc;

    // Create the window
    _pWindow = Window::Create(windowDesc, this);
    _pWindow->setWindowIcon(std::filesystem::current_path() / "Data/Fairy-Tale-Castle-Princess.ico");
}

Application::~Application()
{
    /// 销毁各种部件
    _pWindow.reset();
}

int Application::run()
{
    _runInternal();
    return _returnCode;
}

void Application::renderFrame()
{
    /// 这里执行主要的渲染逻辑
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    _inputState.endFrame();
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

void Application::handleRenderFrame()
{
    renderFrame();
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

void Application::_initUI()
{
}

void Application::_saveConfigToFile()
{
}

void Application::_renderUI()
{
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
