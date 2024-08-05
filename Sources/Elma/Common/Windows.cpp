/**
 * @File Windows.cpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/13
 * @Brief 
 */

#include "Windows.hpp"
#include "./Logger.hpp"
#include "./Inputs.hpp"

#include "glad/include/glad/glad.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

using namespace elma;

class elma::ApiCallbacks
{
public:
    static void windowSizeCallback(GLFWwindow* pGlfwWindow, i32 width, i32 height)
    {
        if (width == 0 || height == 0) {
            return;
        }

        auto* pWindow = (Window*)glfwGetWindowUserPointer(pGlfwWindow);
        if (pWindow != nullptr) {
            pWindow->resize(cast_to<u32>(width), cast_to<u32>(height));
        }
    }

    static void keyboardCallback(GLFWwindow* pGlfwWindow, i32 key, i32 /*scanCode*/, i32 action, i32 modifiers)
    {
        KeyboardEvent event{};
        if (prepareKeyboardEvent(key, action, modifiers, event)) {
            auto* pWindow = (Window*)glfwGetWindowUserPointer(pGlfwWindow);
            if (pWindow != nullptr) {
                pWindow->_pCallbacks->handleKeyboardEvent(event);
            }
        }
    }

    static void charInputCallback(GLFWwindow* pGlfwWindow, u32 input)
    {
        KeyboardEvent event{};
        event.type      = KeyboardEvent::Type::Input;
        event.codepoint = input;

        auto* pWindow = (Window*)glfwGetWindowUserPointer(pGlfwWindow);
        if (pWindow != nullptr) {
            pWindow->_pCallbacks->handleKeyboardEvent(event);
        }
    }

    static void mouseMoveCallback(GLFWwindow* pGlfwWindow, double mouseX, double mouseY)
    {
        auto* pWindow = (Window*)glfwGetWindowUserPointer(pGlfwWindow);
        if (pWindow != nullptr) {
            MouseEvent event;
            event.type       = MouseEvent::Type::Move;
            event.pos        = calcMousePos(mouseX, mouseY, pWindow->_getMouseScale());
            event.screenPos  = {mouseX, mouseY};
            event.wheelDelta = float2(0, 0);

            pWindow->_pCallbacks->handleMouseEvent(event);
        }
    }

    static void mouseButtonCallback(GLFWwindow* pGlfwWindow, i32 button, i32 action, i32 modifiers)
    {
        MouseEvent event;
        // Prepare the mouse data
        MouseEvent::Type type = (action == GLFW_PRESS) ? MouseEvent::Type::ButtonDown : MouseEvent::Type::ButtonUp;
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT :
            event.type   = type;
            event.button = MouseButton::Left;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE :
            event.type   = type;
            event.button = MouseButton::Middle;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT :
            event.type   = type;
            event.button = MouseButton::Right;
            break;
        default :
            // Other keys are not supported
            return;
        }

        auto* pWindow = (Window*)glfwGetWindowUserPointer(pGlfwWindow);
        if (pWindow != nullptr) {
            // Modifiers
            event.mods = getModifierFlags(modifiers);
            double x, y;
            glfwGetCursorPos(pGlfwWindow, &x, &y);
            event.pos = calcMousePos(x, y, pWindow->_getMouseScale());

            pWindow->_pCallbacks->handleMouseEvent(event);
        }
    }

    static void mouseWheelCallback(GLFWwindow* pGlfwWindow, double scrollX, double scrollY)
    {
        auto* pWindow = (Window*)glfwGetWindowUserPointer(pGlfwWindow);
        if (pWindow != nullptr) {
            MouseEvent event;
            event.type = MouseEvent::Type::Wheel;
            double x, y;
            glfwGetCursorPos(pGlfwWindow, &x, &y);
            event.pos        = calcMousePos(x, y, pWindow->_getMouseScale());
            event.wheelDelta = (float2(float(scrollX), float(scrollY)));

            pWindow->_pCallbacks->handleMouseEvent(event);
        }
    }

    static void errorCallback(i32 errorCode, const char* pDescription)
    {
        LogError("GLFW error {}: {}", errorCode, pDescription);
    }

    static void droppedFileCallback(GLFWwindow* pGlfwWindow, i32 count, const char** paths)
    {
        auto* pWindow = (Window*)glfwGetWindowUserPointer(pGlfwWindow);
        if (pWindow) {
            for (i32 i = 0; i < count; i++) {
                std::filesystem::path path(paths[i]);
                pWindow->_pCallbacks->handleDroppedFile(path);
            }
        }
    }

private:
    static inline Key glfwToFalcorKey(i32 glfwKey)
    {
        static_assert(GLFW_KEY_ESCAPE == 256, "GLFW_KEY_ESCAPE 需要是 256");
        static_assert((u32)Key::Escape >= 256, "Key::Escape 至少需要是 256");

        if (glfwKey < GLFW_KEY_ESCAPE) {
            // 与定义相同，直接返回
            return (Key)glfwKey;
        }

        switch (glfwKey) {
        case GLFW_KEY_ESCAPE        : return Key::Escape;
        case GLFW_KEY_ENTER         : return Key::Enter;
        case GLFW_KEY_TAB           : return Key::Tab;
        case GLFW_KEY_BACKSPACE     : return Key::Backspace;
        case GLFW_KEY_INSERT        : return Key::Insert;
        case GLFW_KEY_DELETE        : return Key::Del;
        case GLFW_KEY_RIGHT         : return Key::Right;
        case GLFW_KEY_LEFT          : return Key::Left;
        case GLFW_KEY_DOWN          : return Key::Down;
        case GLFW_KEY_UP            : return Key::Up;
        case GLFW_KEY_PAGE_UP       : return Key::PageUp;
        case GLFW_KEY_PAGE_DOWN     : return Key::PageDown;
        case GLFW_KEY_HOME          : return Key::Home;
        case GLFW_KEY_END           : return Key::End;
        case GLFW_KEY_CAPS_LOCK     : return Key::CapsLock;
        case GLFW_KEY_SCROLL_LOCK   : return Key::ScrollLock;
        case GLFW_KEY_NUM_LOCK      : return Key::NumLock;
        case GLFW_KEY_PRINT_SCREEN  : return Key::PrintScreen;
        case GLFW_KEY_PAUSE         : return Key::Pause;
        case GLFW_KEY_F1            : return Key::F1;
        case GLFW_KEY_F2            : return Key::F2;
        case GLFW_KEY_F3            : return Key::F3;
        case GLFW_KEY_F4            : return Key::F4;
        case GLFW_KEY_F5            : return Key::F5;
        case GLFW_KEY_F6            : return Key::F6;
        case GLFW_KEY_F7            : return Key::F7;
        case GLFW_KEY_F8            : return Key::F8;
        case GLFW_KEY_F9            : return Key::F9;
        case GLFW_KEY_F10           : return Key::F10;
        case GLFW_KEY_F11           : return Key::F11;
        case GLFW_KEY_F12           : return Key::F12;
        case GLFW_KEY_KP_0          : return Key::Keypad0;
        case GLFW_KEY_KP_1          : return Key::Keypad1;
        case GLFW_KEY_KP_2          : return Key::Keypad2;
        case GLFW_KEY_KP_3          : return Key::Keypad3;
        case GLFW_KEY_KP_4          : return Key::Keypad4;
        case GLFW_KEY_KP_5          : return Key::Keypad5;
        case GLFW_KEY_KP_6          : return Key::Keypad6;
        case GLFW_KEY_KP_7          : return Key::Keypad7;
        case GLFW_KEY_KP_8          : return Key::Keypad8;
        case GLFW_KEY_KP_9          : return Key::Keypad9;
        case GLFW_KEY_KP_DECIMAL    : return Key::KeypadDel;
        case GLFW_KEY_KP_DIVIDE     : return Key::KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY   : return Key::KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT   : return Key::KeypadSubtract;
        case GLFW_KEY_KP_ADD        : return Key::KeypadAdd;
        case GLFW_KEY_KP_ENTER      : return Key::KeypadEnter;
        case GLFW_KEY_KP_EQUAL      : return Key::KeypadEqual;
        case GLFW_KEY_LEFT_SHIFT    : return Key::LeftShift;
        case GLFW_KEY_LEFT_CONTROL  : return Key::LeftControl;
        case GLFW_KEY_LEFT_ALT      : return Key::LeftAlt;
        case GLFW_KEY_LEFT_SUPER    : return Key::LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT   : return Key::RightShift;
        case GLFW_KEY_RIGHT_CONTROL : return Key::RightControl;
        case GLFW_KEY_RIGHT_ALT     : return Key::RightAlt;
        case GLFW_KEY_RIGHT_SUPER   : return Key::RightSuper;
        case GLFW_KEY_MENU          : return Key::Menu;
        default                     : return Key::Unknown;
        }
    }

    static inline ModifierFlags getModifierFlags(i32 modifiers)
    {
        ModifierFlags flags = ModifierFlags::None;
        if (modifiers & GLFW_MOD_ALT)
            flags |= ModifierFlags::Alt;
        if (modifiers & GLFW_MOD_CONTROL)
            flags |= ModifierFlags::Ctrl;
        if (modifiers & GLFW_MOD_SHIFT)
            flags |= ModifierFlags::Shift;
        return flags;
    }

    static i32 fixGLFWModifiers(i32 modifiers, i32 key, i32 action)
    {
        i32 bit = 0;
        if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
            bit = GLFW_MOD_SHIFT;
        if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
            bit = GLFW_MOD_CONTROL;
        if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT)
            bit = GLFW_MOD_ALT;
        return (action == GLFW_RELEASE) ? modifiers & (~bit) : modifiers | bit;
    }

    static inline float2 calcMousePos(double xPos, double yPos, const float2& mouseScale)
    {
        auto pos  = float2(float(xPos), float(yPos));
        pos      *= mouseScale;
        return pos;
    }

    static inline bool prepareKeyboardEvent(i32 key, i32 action, i32 modifiers, KeyboardEvent& event)
    {
        if (key == GLFW_KEY_UNKNOWN) {
            return false;
        }

        modifiers = fixGLFWModifiers(modifiers, key, action);

        switch (action) {
        case GLFW_RELEASE : event.type = KeyboardEvent::Type::KeyReleased; break;
        case GLFW_PRESS   : event.type = KeyboardEvent::Type::KeyPressed; break;
        case GLFW_REPEAT  : event.type = KeyboardEvent::Type::KeyRepeated; break;
        default           : ELMA_UNREACHABLE();
        }
        event.key  = glfwToFalcorKey(key);
        event.mods = getModifierFlags(modifiers);

        return true;
    }
};

namespace {

static std::atomic<size_t> sWindowCount = 0;

} // namespace

Ref<Window> Window::Create(const Desc& desc, ICallbacks* pCallbacks)
{
    return Ref<Window>(new Window(desc, pCallbacks));
}

Window::Window(const Desc& desc, ICallbacks* pCallbacks)
: _desc(desc), _mouseScale(1.0f / (float)desc.width, 1.0f / (float)desc.height), _pCallbacks(pCallbacks)
{
    glfwSetErrorCallback(ApiCallbacks::errorCallback);

    if (sWindowCount.fetch_add(1, std::memory_order_relaxed) == 0) {
        ELMA_CHECK(glfwInit() != GLFW_FALSE, "GLFW 初始化失败.");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    //    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    u32 w = desc.width;
    u32 h = desc.height;

    if (!desc.resizableWindow) {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    }

    _pGLFWWindow = glfwCreateWindow(static_cast<i32>(w), static_cast<i32>(h), desc.title.data(), nullptr, nullptr);

    glfwMakeContextCurrent(_pGLFWWindow);
    glfwSwapInterval(0);

    ELMA_ASSERT(_pGLFWWindow, "创建 GLFW 窗口失败.");

    glfwSetWindowPos(_pGLFWWindow, 5, 30);

    _apiHandle = glfwGetWin32Window(_pGLFWWindow);
    ELMA_ASSERT(_apiHandle, "无法获取 Native win32 handle.");

    _updateWindowSize();

    glfwSetWindowUserPointer(_pGLFWWindow, this);

    // 设置回调
    glfwSetKeyCallback(_pGLFWWindow, ApiCallbacks::keyboardCallback);
    glfwSetCursorPosCallback(_pGLFWWindow, ApiCallbacks::mouseMoveCallback);
    glfwSetMouseButtonCallback(_pGLFWWindow, ApiCallbacks::mouseButtonCallback);
    glfwSetCharCallback(_pGLFWWindow, ApiCallbacks::charInputCallback);
    glfwSetScrollCallback(_pGLFWWindow, ApiCallbacks::mouseWheelCallback);
    glfwSetWindowSizeCallback(_pGLFWWindow, ApiCallbacks::windowSizeCallback);
    glfwSetDropCallback(_pGLFWWindow, ApiCallbacks::droppedFileCallback);

    glfwShowWindow(_pGLFWWindow);
    glfwFocusWindow(_pGLFWWindow);

    ELMA_CHECK(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "glad 初始化失败.");
}

Window::~Window()
{
    // Cleanup
    glfwDestroyWindow(_pGLFWWindow);

    if (sWindowCount.fetch_sub(1) == 1)
        glfwTerminate();
}

void Window::_updateWindowSize()
{
    i32 width, height;
    glfwGetWindowSize(_pGLFWWindow, &width, &height);
    _setWindowSize(width, height);
}

void Window::_setWindowSize(i32 width, i32 height)
{
    ELMA_ASSERT(width > 0 && height > 0, "无效的窗口尺寸.");

    _desc.width   = cast_to<u32>(width);
    _desc.height  = cast_to<u32>(height);
    _mouseScale.x = 1.0f / (float)_desc.width;
    _mouseScale.y = 1.0f / (float)_desc.height;
}

void Window::shutdown()
{
    glfwSetWindowShouldClose(_pGLFWWindow, 1);
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(_pGLFWWindow);
}

void Window::resize(u32 width, u32 height)
{
    glfwSetWindowSize(_pGLFWWindow, cast_to<i32>(width), cast_to<i32>(height));

    _updateWindowSize();

    _pCallbacks->handleWindowSizeChange();
}

void Window::msgLoop()
{
    _pCallbacks->handleWindowSizeChange();

    while (!shouldClose()) {
        pollForEvents();

        _pCallbacks->handleRenderFrame();

        glfwSwapBuffers(_pGLFWWindow);
    }
}

void Window::setWindowPos(int32_t x, int32_t y)
{
    glfwSetWindowPos(_pGLFWWindow, x, y);
}

void Window::setWindowTitle(const std::string& title)
{
    glfwSetWindowTitle(_pGLFWWindow, title.c_str());
}

void Window::setWindowIcon(const std::filesystem::path& path)
{
    auto hIcon =
        LoadImageW(GetModuleHandleW(nullptr), path.c_str(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);

    if (!hIcon) {
        LogError("载入 icon '{}' 失败.", path.string());
        return;
    }
    HWND hWnd = _apiHandle ? static_cast<HWND>(_apiHandle) : GetActiveWindow();
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
}

void Window::pollForEvents()
{
    glfwPollEvents();
}
