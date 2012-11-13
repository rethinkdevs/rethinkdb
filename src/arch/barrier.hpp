#ifndef ARCH_BARRIER_HPP_
#define ARCH_BARRIER_HPP_

#include <pthread.h>

#include "errors.hpp"

// We call this a pthread_barrier_t so as to differentiate from other barrier types.
class thread_barrier_t {
public:
    thread_barrier_t(int num_workers);
    ~thread_barrier_t();

    void wait();

private:
    // TODO(OSX) find a better way to detect pthread_barrier_t feature.
#if __APPLE__
    const int num_workers_;
    int num_waiters_;
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;
#else
    pthread_barrier_t barrier_;
#endif

    DISABLE_COPYING(thread_barrier_t);
};

#endif  // ARCH_BARRIER_HPP_
