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

namespace IceInternal
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
	++_ref;
    }

    void __decRef()
    {
	if(--_ref == 0)
	    delete this;
    }

private:

    int _ref;
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
	_mutex.lock();
	++_ref;
	_mutex.unlock();
    }

    void __decRef()
    {
	_mutex.lock();
	bool doDelete = false;
	if(--_ref == 0)
	    doDelete = true;
	_mutex.unlock();
	if(doDelete)
	    delete this;
    }

private:

    int _ref;
    JTCMutex _mutex;
};

}

#endif
