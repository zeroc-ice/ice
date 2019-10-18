//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_COUNT_DOWN_LATCH_H
#define ICE_UTIL_COUNT_DOWN_LATCH_H

#include <IceUtil/Config.h>

namespace IceUtilInternal
{

//
// See java.util.concurrent.CountDownLatch in Java 1.5
//

class ICE_API CountDownLatch
{
public:

    CountDownLatch(int);
    ~CountDownLatch();

    void await() const;
    void countDown();
    int getCount() const;

private:

#ifdef _WIN32
    mutable LONG _count;
    HANDLE _event;
#else
    int _count;
    mutable pthread_mutex_t _mutex;
    mutable pthread_cond_t _cond;

    inline void lock() const;
    inline void unlock() const;
#endif
};

}

#endif
