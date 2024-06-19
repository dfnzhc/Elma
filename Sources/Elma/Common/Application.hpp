/**
 * @File Application.hpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/15
 * @Brief This file is part of Nova.
 */

#pragma once

#include "./Windows.hpp"
#include "./Inputs.hpp"

namespace elma {

struct AppConfig
{
    Window::Desc windowDesc; ///< Window settings.

    f32 timeScale  = 1.0f;
    bool pauseTime = false;
    bool showUI    = true;
};

class Application : public Window::ICallbacks
{
public:
    explicit Application(const AppConfig& config);
    ~Application();

    int run();

    virtual void onLoad() { }

    virtual void onShutdown() { }

    virtual bool onKeyEvent(const KeyboardEvent& /*keyEvent*/) { return false; }

    virtual bool onMouseEvent(const MouseEvent& /*mouseEvent*/) { return false; }

    virtual void onDroppedFile(const std::filesystem::path& /*path*/) { }

    Window* getWindow() { return _pWindow.get(); }

    void renderFrame();

    const InputState& getInputState() { return _inputState; }

    void shutdown(int returnCode = 0);

    AppConfig getConfig() const;

    void toggleVsync(bool on) { _vsyncOn = on; }

    bool isVsyncEnabled() const { return _vsyncOn; }

    static std::string getKeyboardShortcutsStr();

private:
    void handleRenderFrame() override;
    void handleKeyboardEvent(const KeyboardEvent& keyEvent) override;
    void handleMouseEvent(const MouseEvent& mouseEvent) override;
    void handleDroppedFile(const std::filesystem::path& path) override;

    void _initUI();
    void _saveConfigToFile();

    void _renderUI();
    void _runInternal();

    std::unique_ptr<Window> _pWindow = nullptr;

    InputState _inputState;

    bool _shouldTerminate = false;
    bool _vsyncOn         = false;
    bool _showUI          = true;

    int _returnCode = 0;
};
}; // namespace elma
