// **********************************************************************
//
// Copyright (c) 2004
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_UTIL_COUNT_DOWN_LATCH_H
#define ICE_UTIL_COUNT_DOWN_LATCH_H

#include <IceUtil/Config.h>

namespace IceUtil
{

//
// See java.util.concurrent.CountDownLatch in Java 1.5
//

class ICE_UTIL_API CountDownLatch
{
public:
    
    CountDownLatch(int);

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
