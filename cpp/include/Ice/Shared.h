// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SHARED_H
#define ICE_SHARED_H

#include <Ice/Config.h>

namespace __Ice
{

//
// Simple, non thread-safe intrusive base class for reference-counted
// types.
//
class ICE_API SimpleShared : noncopyable
{
public:

    SimpleShared();
    virtual ~SimpleShared();

    void __incRef()
    {
	++ref_;
    }

    void __decRef()
    {
	if(--ref_ == 0)
	    delete this;
    }

private:

    int ref_;
};

//
// Thread-safe intrusive base class for reference-counted types.
//
class ICE_API Shared : noncopyable
{
public:

    Shared();
    virtual ~Shared();

    void __incRef()
    {
	mutex_.lock();
	++ref_;
	mutex_.unlock();
    }

    void __decRef()
    {
	mutex_.lock();
	bool doDelete = false;
	if(--ref_ == 0)
	    doDelete = true;
	mutex_.unlock();
	if(doDelete)
	    delete this;
    }

private:

    int ref_;
    JTCMutex mutex_;
};

}

#endif
