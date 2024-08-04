/**
 * @File Object.hpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/13
 * @Brief 
 */

#pragma once

#include <atomic>
#include "./Defines.hpp"

namespace elma {

class Object
{
public:
    Object() = default;

    Object(const Object&) { }

    void incRef() const;
    void decRef(bool dealloc = true) const noexcept;

    void traceRefs() const;
    static void traceAliveObjects();

    int refCount() const { return static_cast<int>(_refCount); }

    virtual std::string_view id() const { return "Object"; }

protected:
    virtual ~Object() = default;

private:
    mutable std::atomic_uint32_t _refCount{0};
};

#define NOVA_OBJECT(class_)                                                                                            \
public:                                                                                                                \
    std::string_view id() const override { return #class_; }

static inline uint64_t NextRefId()
{
    static std::atomic<uint64_t> sNextId = 0;
    return sNextId.fetch_add(1);
}

static constexpr uint64_t InvalidRefId = static_cast<uint64_t>(-1);

template<typename T> class Ref
{
public:
    static_assert(std::derived_from<T, Object>, "需要是 Object 的派生类型.");

    // -------------------------
    // 构造与析构函数
    // -------------------------
    Ref() = default;

    explicit Ref(std::nullptr_t) { }

    template<typename U = T>
        requires std::convertible_to<U*, T*>
    explicit Ref(U* ptr) : _ptr(ptr)
    {
        if (_ptr)
            incRef(static_cast<const Object*>(_ptr));
    }

    Ref(const Ref& r) : _ptr(r._ptr)
    {
        if (_ptr)
            incRef(static_cast<const Object*>(_ptr));
    }

    template<typename U = T>
        requires std::convertible_to<U*, T*>
    explicit Ref(const Ref<U>& r) : _ptr(r._ptr)
    {
        if (_ptr)
            incRef(static_cast<const Object*>(_ptr));
    }

    Ref(Ref&& r) noexcept : _ptr(r._ptr) { r._ptr = nullptr; }

    template<typename U = T>
        requires std::convertible_to<U*, T*>
    explicit Ref(const Ref<U>&& r) : _ptr(r._ptr)
    {
        r._ptr = nullptr;
    }

    ~Ref()
    {
        if (_ptr)
            decRef(static_cast<const Object*>(_ptr));
    }

    // -------------------------
    // 拷贝函数
    // -------------------------

    Ref& operator=(const Ref& r) noexcept
    {
        if (r != *this) {
            if (r._ptr)
                incRef(static_cast<const Object*>(r._ptr));
            if (auto oldPtr = std::exchange(_ptr, r._ptr); oldPtr)
                decRef(static_cast<const Object*>(oldPtr));
        }

        return *this;
    }

    template<typename U = T>
        requires std::convertible_to<U*, T*>
    Ref& operator=(const Ref<U>& r) noexcept
    {
        if (r != *this) {
            if (r._ptr)
                incRef(static_cast<const Object*>(r._ptr));
            if (auto oldPtr = std::exchange(_ptr, r._ptr); oldPtr)
                decRef(static_cast<const Object*>(oldPtr));
        }

        return *this;
    }

    Ref& operator=(Ref&& r) noexcept
    {
        if (static_cast<void*>(&r) != this) {
            if (_ptr)
                decRef(static_cast<const Object*>(_ptr));
            _ptr   = r._ptr;
            r._ptr = nullptr;
        }

        return *this;
    }

    template<typename U = T>
        requires std::convertible_to<U*, T*>
    Ref& operator=(Ref<U>&& r) noexcept
    {
        if (static_cast<void*>(&r) != this) {
            if (_ptr)
                decRef(static_cast<const Object*>(_ptr));
            _ptr   = r._ptr;
            r._ptr = nullptr;
        }

        return *this;
    }

    // -------------------------
    // 一些成员函数
    // -------------------------
    template<typename U = T>
        requires std::convertible_to<U*, T*>
    bool operator==(const U* ptr) const
    {
        return _ptr == ptr;
    }

    template<typename U = T>
        requires std::convertible_to<U*, T*>
    bool operator!=(const U* ptr) const
    {
        return _ptr != ptr;
    }

    template<typename U = T>
        requires std::convertible_to<U*, T*>
    auto operator<=>(const U* ptr) const
    {
        return _ptr <=> ptr;
    }

    template<typename U = T>
        requires(std::convertible_to<U*, T*> or std::convertible_to<T*, U*>)
    bool operator==(const Ref<U>& r) const
    {
        return _ptr == r._ptr;
    }

    template<typename U = T>
        requires(std::convertible_to<U*, T*> or std::convertible_to<T*, U*>)
    bool operator!=(const Ref<U>& r) const
    {
        return _ptr != r._ptr;
    }

    template<typename U = T>
        requires(std::convertible_to<U*, T*> or std::convertible_to<T*, U*>)
    auto operator<=>(const Ref<U>& r) const
    {
        return _ptr <=> r._ptr;
    }

    // clang-format off
    bool operator==(std::nullptr_t)  const { return _ptr == nullptr; }
    bool operator!=(std::nullptr_t)  const { return _ptr != nullptr; }
    auto operator<=>(std::nullptr_t) const { return _ptr <=> nullptr; }

    T*               get() const { return _ptr; }
    T&         operator*() const { return *_ptr; }
    T*        operator->() const { return _ptr; }
    explicit operator T*() const { return _ptr; }
    // clang-format on

    explicit operator bool() const { return _ptr != nullptr; }

    void swap(Ref& r) noexcept { std::swap(_ptr, r._ptr); }

    template<typename U = T>
        requires std::convertible_to<U*, T*>
    void reset(U* ptr = nullptr) noexcept
    {
        if (ptr != _ptr) {
            if (ptr)
                incRef(static_cast<const Object*>(ptr));
            if (auto oldPtr = std::exchange(_ptr, ptr); oldPtr)
                decRef(static_cast<const Object*>(oldPtr));
        }
    }

private:
    inline void incRef(const Object* object) { object->incRef(); }
    inline void decRef(const Object* object) { object->decRef(); }

    T* _ptr = nullptr;
    template<typename U> friend class Ref;
};

template<typename T> class WeakRef
{
public:
    static_assert(std::derived_from<T, Object>, "需要是 Object 的派生类型");

    explicit WeakRef(const Ref<T>& r) : _ref(r), _weakRef(_ref.get()) { }
    explicit WeakRef(Ref<T>&& r) : _ref(r), _weakRef(_ref.get()) { }

    WeakRef()                         = delete;
    WeakRef& operator=(const Ref<T>&) = delete;
    WeakRef& operator=(Ref<T>&&)      = delete;

    T* get() const { return _weakRef; }

    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }

    explicit operator Ref<T>() const { return ref<T>(get()); }
    explicit operator T*() const { return get(); }
    explicit operator bool() const { return get() != nullptr; }

    void release() { _ref.reset(); }

private:
    Ref<T> _ref;
    T* _weakRef = nullptr;
};

} // namespace elma