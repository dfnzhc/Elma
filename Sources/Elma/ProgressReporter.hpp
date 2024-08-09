#pragma once

#include "Elma.hpp"
#include <mutex>

namespace elma {
/// For printing how much work is done for an operation.
/// The operations are thread-safe so we can safely use this in multi-thread environment.
class ProgressReporter
{
public:
    ProgressReporter(uint64_t total_work) : totalWork(total_work), workDone(0) { }

    void update(uint64_t num)
    {
        std::lock_guard<std::mutex> lock(mutex);
        workDone        += num;
        Real work_ratio  = (Real)workDone / (Real)totalWork;
        //        fprintf(stdout,
        //                "\r %.2f Percent Done (%llu / %llu)",
        //                work_ratio * Real(100.0),
        //                (unsigned long long)work_done,
        //                (unsigned long long)total_work);
    }

    void done()
    {
        workDone = totalWork;
        //        fprintf(stdout,
        //                "\r %.2f Percent Done (%llu / %llu)\n",
        //                Real(100.0),
        //                (unsigned long long)work_done,
        //                (unsigned long long)total_work);
    }

    uint64_t getWorkDone() const { return workDone; }

private:
    const uint64_t totalWork;
    uint64_t workDone;
    std::mutex mutex;
};

} // namespace elma