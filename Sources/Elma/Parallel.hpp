#pragma once

#include "Vector.hpp"

#include <mutex>
#include <functional>
#include <atomic>

namespace elma {
// From https://github.com/mmp/pbrt-v3/blob/master/src/core/parallel.h
extern thread_local int ThreadIndex;

void ParallelFor(const std::function<void(int64_t)>& func, int64_t count, int64_t chunk_size = 1);
void ParallelFor(std::function<void(Vector2i)> func, const Vector2i count);

void ParallelInit(int num_threads);
void ParallelCleanup();

} // namespace elma