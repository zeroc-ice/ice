// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_THREAD_POOL_H
#define ICE_THREAD_POOL_H

#include <Ice/ThreadPoolF.h>
#include <Ice/InstanceF.h>
#include <Ice/Shared.h>

namespace _Ice
{

class ICE_API ThreadPoolI : public Shared, public JTCMutex
{
public:
    
    
private:

    ThreadPoolI(const ThreadPoolI&);
    void operator=(const ThreadPoolI&);

    ThreadPoolI(const Instance&);
    virtual ~ThreadPoolI();
    void destroy();
    friend class InstanceI; // May create and destroy ThreadPoolIs

    Instance instance_;
};

}

#endif
