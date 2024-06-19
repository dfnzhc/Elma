/**
 * @File Windows.hpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/13
 * @Brief 
 */

#pragma once

#include <filesystem>
#include <string_view>
#include "./Defines.hpp"

struct GLFWwindow;

namespace elma {

struct KeyboardEvent;
struct MouseEvent;

class Window
{
public:
    using NativeHandle = void*;

    struct Desc
    {
        uint32_t width         = 960;               ///< 客户端区域的宽度。
        uint32_t height        = 720;               ///< 客户端区域的高度。
        std::string_view title = "Elma Renderer";   ///< 窗口标题。
    };

    /**
     * 创建新对象时使用的回调接口
     */
    class ICallbacks
    {
    public:
        virtual ~ICallbacks()                                             = default;
        virtual void handleRenderFrame()                                  = 0;
        virtual void handleKeyboardEvent(const KeyboardEvent& keyEvent)   = 0;
        virtual void handleMouseEvent(const MouseEvent& mouseEvent)       = 0;
        virtual void handleDroppedFile(const std::filesystem::path& path) = 0;
    };

    static std::unique_ptr<Window> Create(const Desc& desc, ICallbacks* pCallbacks);

    ~Window();

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    void shutdown();
    bool shouldClose() const;

    void msgLoop();

    void pollForEvents();

    void setWindowPos(int32_t x, int32_t y);

    void setWindowTitle(const std::string& title);

    void setWindowIcon(const std::filesystem::path& path);

    const NativeHandle& getApiHandle() const { return _apiHandle; }

    uint2 getClientAreaSize() const { return {_desc.width, _desc.height}; }

    const Desc& getDesc() const { return _desc; }

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