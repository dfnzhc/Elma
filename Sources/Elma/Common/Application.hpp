/**
 * @File Application.hpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/15
 * @Brief This file is part of Nova.
 */

#pragma once

#include "./Windows.hpp"
#include "Inputs.hpp"

namespace elma {

struct AppConfig
{
    Window::Desc windowDesc; ///< Window settings.

    bool headless  = false;
    f32 timeScale  = 1.0f;
    bool pauseTime = false;
    bool showUI    = true;

    bool generateShaderDebugInfo = false;
    bool shaderPreciseFloat      = false;
};

class Application : public Window::ICallbacks
{
public:
    explicit Application(const AppConfig& config);
    ~Application() override;

    int run();

    virtual void onLoad() { }

    virtual void onShutdown() { }

    virtual void onResize(uint32_t /*width*/, uint32_t /*height*/) { }

    virtual void onFrameRender() { }

    //    virtual void onGuiRender(Gui* pGui){};

    virtual void onOptionsChange() { }

    virtual void onHotReload() { }

    virtual bool onKeyEvent(const KeyboardEvent& /*keyEvent*/) { return false; }

    virtual bool onMouseEvent(const MouseEvent& /*mouseEvent*/) { return false; }

    virtual void onDroppedFile(const std::filesystem::path& /*path*/) { }

    Window* getWindow() { return _pWindow.get(); }

    void resizeFrameBuffer(uint32_t width, uint32_t height);

    void renderFrame();

    const InputState& getInputState() { return _inputState; }

    void shutdown(int returnCode = 0);

    AppConfig getConfig() const;

    void toggleVsync(bool on) { _vsyncOn = on; }

    bool isVsyncEnabled() const { return _vsyncOn; }

    static std::string getKeyboardShortcutsStr();

private:
    void handleWindowSizeChange() override;
    void handleRenderFrame() override;
    void handleKeyboardEvent(const KeyboardEvent& keyEvent) override;
    void handleMouseEvent(const MouseEvent& mouseEvent) override;
    void handleDroppedFile(const std::filesystem::path& path) override;

    void _resizeTargetFBO(uint32_t width, uint32_t height);
    void _initUI();
    void _saveConfigToFile();

    void _renderUI();
    void _runInternal();

    Ref<Window> _pWindow;

    InputState _inputState;

    bool _shouldTerminate = false;
    bool _vsyncOn         = false;
    bool _showUI          = true;

    int _returnCode = 0;
};
}; // namespace elma
