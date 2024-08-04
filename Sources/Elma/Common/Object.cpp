/**
 * @File Object.cpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/6/13
 * @Brief 
 */

#include <mutex>
#include <set>
#include "./Object.hpp"
#include "./Error.hpp"
#include "./Logger.hpp"

using namespace elma;

namespace {

std::mutex sTrackedObjectsMutex;
std::set<const Object*> sTrackedObjects;

} // namespace

void Object::incRef() const
{
    auto refCount = _refCount.fetch_add(1);
    if (refCount == 0) {
        auto lock = std::lock_guard(sTrackedObjectsMutex);
        sTrackedObjects.insert(this);
    }
}

void Object::decRef(bool dealloc) const noexcept
{
    auto refCount = _refCount.fetch_sub(1);
    if (refCount <= 0) [[unlikely]]
        ELMA_UNREACHABLE();

    else if (refCount == 1) {
        {
            auto lock = std::lock_guard(sTrackedObjectsMutex);
            sTrackedObjects.erase(this);
        }

        if (dealloc)
            delete this;
    }
}

void Object::traceRefs() const
{
    LogInfo("对象 (类 = {}, 地址 = {:#010x}) 有 {} 个引用", id(), reinterpret_cast<std::uintptr_t>(this), refCount());
}

void Object::traceAliveObjects()
{
    auto lock = std::lock_guard(sTrackedObjectsMutex);
    LogInfo("共有 ({}) 对象存活：", sTrackedObjects.size());
    for (const auto* object : sTrackedObjects)
        object->traceRefs();
}
