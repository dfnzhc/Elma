/**
 * @File Windows.hpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/13
 * @Brief 
 */

#pragma once

#include <filesystem>
#include <string_view>
#include "./Object.hpp"

struct GLFWwindow;

namespace elma {

struct KeyboardEvent;
struct MouseEvent;

class Window : public Object
{
    NOVA_OBJECT(Window)
public:
    using NativeHandle = void*;

    /**
     * 窗口模式
     */
    enum class WindowMode
    {
        Normal,     ///< 普通窗口。
        Minimized,  ///< 最小化窗口。
        FullScreen, ///< 全屏窗口。
    };

    /**
     * 窗口配置
     */
    struct Desc
    {
        uint32_t width         = 1'920;           ///< 客户端区域的宽度。
        uint32_t height        = 1'080;           ///< 客户端区域的高度。
        std::string_view title = "Nova Renderer"; ///< 窗口标题。
        WindowMode mode = WindowMode::Normal; ///< 窗口模式。在全屏模式下，宽度和高度将被忽略。
        bool resizableWindow = true;          ///< 允许用户调整窗口大小。
        bool enableVSync     = false;         ///< 控制垂直同步。
    };

    /**
     * 创建新对象时使用的回调接口
     */
    class ICallbacks
    {
    public:
        virtual ~ICallbacks()                                             = default;
        virtual void handleWindowSizeChange()                             = 0;
        virtual void handleRenderFrame()                                  = 0;
        virtual void handleKeyboardEvent(const KeyboardEvent& keyEvent)   = 0;
        virtual void handleMouseEvent(const MouseEvent& mouseEvent)       = 0;
        virtual void handleDroppedFile(const std::filesystem::path& path) = 0;
    };

    static Ref<Window> Create(const Desc& desc, ICallbacks* pCallbacks);

    ~Window() override;

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    void shutdown();
    bool shouldClose() const;

    void resize(u32 width, u32 height);

    void msgLoop();

    void pollForEvents();

    void setWindowPos(int32_t x, int32_t y);

    void setWindowTitle(const std::string& title);

    void setWindowIcon(const std::filesystem::path& path);

    const NativeHandle& getApiHandle() const { return _apiHandle; }

    uint2 getClientAreaSize() const { return {_desc.width, _desc.height}; }

    const Desc& getDesc() const { return _desc; }

    GLFWwindow* getHandle() const { return _pGLFWWindow; }

private:
    friend class ApiCallbacks;
    Window(const Desc& desc, ICallbacks* pCallbacks);

    void _updateWindowSize();
    void _setWindowSize(i32 width, i32 height);

    Desc _desc;
    GLFWwindow* _pGLFWWindow = nullptr;
    NativeHandle _apiHandle;
    float2 _mouseScale;
    ICallbacks* _pCallbacks = nullptr;

    const float2& _getMouseScale() const { return _mouseScale; }
};

} // namespace elma