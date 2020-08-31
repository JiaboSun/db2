//
// Created by rrzhang on 2019/9/10.
//

#ifndef COMPARE_GLUSTER_AND_GPRC_PROFILER_H
#define COMPARE_GLUSTER_AND_GPRC_PROFILER_H

#include <chrono>

namespace dbx1000 {
    class Profiler {
    public:
        Profiler() {
            finished_ = false;
        }

        Profiler(const Profiler &) = delete;

        Profiler &operator=(const Profiler &) = delete;

        //! Starts the timer
        void Start() {
            finished_ = false;
            start_ = Tick();
        }

//        void ReStart() {
//            finished_ = false;
//        }

        //! Finishes timing
        void End() {
//            assert(false == finished_);
            end_ = Tick();
            finished_ = true;
            count_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_);
        }

        //! Returns the elapsed time in seconds. If End() has been called, returns
        //! the total elapsed time. Otherwise returns how far along the timer is
        //! right now.
        Profiler &Elapsed() {
            if (!finished_) {
                End();
            }
            return *this;
        }

        uint64_t Nanos() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(count_).count();
        }

        uint64_t Micros() {
            return std::chrono::duration_cast<std::chrono::microseconds>(count_).count();
        }

        uint64_t Millis() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(count_).count();
        }

        uint64_t Seconds() {
            return std::chrono::duration_cast<std::chrono::seconds>(count_).count();
        }

        void Clear() {
            count_ = std::chrono::nanoseconds(0);
        }

    private:
        std::chrono::time_point<std::chrono::system_clock> Tick() const {
            return std::chrono::system_clock::now();
        }

        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> start_;
        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> end_;
        bool finished_ = false;
        std::chrono::nanoseconds count_ = std::chrono::nanoseconds(0);
    };
}
#endif //TEST_SOCKET_PROFILER_H
