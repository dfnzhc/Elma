/**
 * @File Inputs.hpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/15
 * @Brief This file is part of Elma.
 */

#pragma once

#include <bitset>
#include "./Enums.hpp"
#include "./Error.hpp"

namespace elma {

enum class MouseButton : u32
{
    Left,
    Middle,
    Right,
    Count
};

enum class Key : u32
{
    Space        = ' ',
    Apostrophe   = '\'',
    Comma        = ',',
    Minus        = '-',
    Period       = '.',
    Slash        = '/',
    Key0         = '0',
    Key1         = '1',
    Key2         = '2',
    Key3         = '3',
    Key4         = '4',
    Key5         = '5',
    Key6         = '6',
    Key7         = '7',
    Key8         = '8',
    Key9         = '9',
    Semicolon    = ';',
    Equal        = '=',
    A            = 'A',
    B            = 'B',
    C            = 'C',
    D            = 'D',
    E            = 'E',
    F            = 'F',
    G            = 'G',
    H            = 'H',
    I            = 'I',
    J            = 'J',
    K            = 'K',
    L            = 'L',
    M            = 'M',
    N            = 'N',
    O            = 'O',
    P            = 'P',
    Q            = 'Q',
    R            = 'R',
    S            = 'S',
    T            = 'T',
    U            = 'U',
    V            = 'V',
    W            = 'W',
    X            = 'X',
    Y            = 'Y',
    Z            = 'Z',
    LeftBracket  = '[',
    Backslash    = '\\',
    RightBracket = ']',
    GraveAccent  = '`',

    Escape = 256,
    Tab,
    Enter,
    Backspace,
    Insert,
    Del,
    Right,
    Left,
    Down,
    Up,
    PageUp,
    PageDown,
    Home,
    End,
    CapsLock,
    ScrollLock,
    NumLock,
    PrintScreen,
    Pause,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    Keypad0,
    Keypad1,
    Keypad2,
    Keypad3,
    Keypad4,
    Keypad5,
    Keypad6,
    Keypad7,
    Keypad8,
    Keypad9,
    KeypadDel,
    KeypadDivide,
    KeypadMultiply,
    KeypadSubtract,
    KeypadAdd,
    KeypadEnter,
    KeypadEqual,
    LeftShift,
    LeftControl,
    LeftAlt,
    LeftSuper, // Windows key on windows
    RightShift,
    RightControl,
    RightAlt,
    RightSuper, // Windows key on windows
    Menu,
    Unknown,    // Any unknown key code

    Count,
};

enum class ModifierFlags : u32
{
    None  = 0,
    Shift = 1,
    Ctrl  = 2,
    Alt   = 4
};

ELMA_ENUM_BIT_OPERATORS(ModifierFlags);

enum class Modifier : u32
{
    Shift = cast_to<u32>(ModifierFlags::Shift),
    Ctrl  = cast_to<u32>(ModifierFlags::Ctrl),
    Alt   = cast_to<u32>(ModifierFlags::Alt)
};

struct MouseEvent
{
    enum class Type
    {
        ButtonDown,
        ButtonUp,
        Move,
        Wheel,
    };

    Type type;
    float2 pos;         // 标准化坐标，范围[0, 1]
    float2 screenPos;   // 屏幕空间坐标，范围[0, 窗口大小]
    float2 wheelDelta;  // 鼠标滚轮滚动量
    ModifierFlags mods; // 键盘修饰键标记
    MouseButton button; // 鼠标按钮
};

struct KeyboardEvent
{
    enum class Type
    {
        KeyPressed,
        KeyReleased,
        KeyRepeated,
        Input,
    };

    Type type;          // 事件类型
    Key key;            // 按下/释放的键
    ModifierFlags mods; // 键盘修饰键标记
    u32 codepoint = 0;  // UTF-32 码点

    bool hasModifier(Modifier mod) const { return isSet(mods, cast_to<ModifierFlags>(mod)); }
};

class InputState
{
public:
    bool isMouseMoving() const { return _mouseMoving; }

    bool isKeyDown(Key key) const { return _currentKeyState[cast_to<size_t>(key)]; }

    bool isKeyPressed(Key key) const
    {
        return _currentKeyState[cast_to<size_t>(key)] && !_previousKeyState[cast_to<u32>(key)];
    }

    bool isKeyReleased(Key key) const
    {
        return !_currentKeyState[cast_to<size_t>(key)] && _previousKeyState[cast_to<u32>(key)];
    }

    bool isMouseButtonDown(MouseButton mb) const { return _currentMouseState[cast_to<size_t>(mb)]; }

    bool isMouseButtonClicked(MouseButton mb) const
    {
        return _currentMouseState[cast_to<size_t>(mb)] && !_previousMouseState[cast_to<u32>(mb)];
    }

    bool isMouseButtonReleased(MouseButton mb) const
    {
        return !_currentMouseState[cast_to<size_t>(mb)] && _previousMouseState[cast_to<u32>(mb)];
    }

    bool isModifierDown(Modifier mod) const { return getModifierState(_currentKeyState, mod); }

    bool isModifierPressed(Modifier mod) const
    {
        return getModifierState(_currentKeyState, mod) && !getModifierState(_previousKeyState, mod);
    }

    bool isModifierReleased(Modifier mod) const
    {
        return !getModifierState(_currentKeyState, mod) && getModifierState(_previousKeyState, mod);
    }

private:
    static constexpr size_t kKeyCount         = cast_to<size_t>(Key::Count);
    static constexpr size_t kMouseButtonCount = cast_to<size_t>(MouseButton::Count);

    using KeyStates  = std::bitset<kKeyCount>;
    using MouseState = std::bitset<kMouseButtonCount>;

    void onKeyEvent(const KeyboardEvent& keyEvent)
    {
        if (keyEvent.type == KeyboardEvent::Type::KeyPressed || keyEvent.type == KeyboardEvent::Type::KeyReleased) {
            _currentKeyState[cast_to<size_t>(keyEvent.key)] = keyEvent.type == KeyboardEvent::Type::KeyPressed;
        }
    }

    void onMouseEvent(const MouseEvent& mouseEvent)
    {
        if (mouseEvent.type == MouseEvent::Type::ButtonDown || mouseEvent.type == MouseEvent::Type::ButtonUp) {
            _currentMouseState[cast_to<size_t>(mouseEvent.button)] = mouseEvent.type == MouseEvent::Type::ButtonDown;
        }
        else if (mouseEvent.type == MouseEvent::Type::Move) {
            _mouseMoving = true;
        }
    }

    void endFrame()
    {
        _previousKeyState   = _currentKeyState;
        _previousMouseState = _currentMouseState;

        _mouseMoving = false;
    }

    bool getModifierState(const KeyStates& states, Modifier mod) const
    {
        switch (mod) {
        case Modifier::Shift :
            return states[cast_to<size_t>(Key::LeftShift)] || states[cast_to<size_t>(Key::RightShift)];
        case Modifier::Ctrl :
            return states[cast_to<size_t>(Key::LeftControl)] || states[cast_to<size_t>(Key::RightControl)];
        case Modifier::Alt : return states[cast_to<size_t>(Key::LeftAlt)] || states[cast_to<size_t>(Key::RightAlt)];
        }
        ELMA_UNREACHABLE();
    }

    KeyStates _currentKeyState;
    KeyStates _previousKeyState;
    MouseState _currentMouseState;
    MouseState _previousMouseState;

    bool _mouseMoving = false;
    friend class Application;
};
} // namespace elma