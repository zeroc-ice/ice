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
	if (--_ref == 0)
	{
	    if(!_noDelete)
	    {
		delete this;
	    }
	}
    }

    int __getRef()
    {
	return _ref;
    }

    void __setNoDelete(bool b)
    {
	_noDelete = b;
    }

private:

    int _ref;
    bool _noDelete;
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
	if (--_ref == 0)
	{
	    doDelete = !_noDelete;
	}
	_mutex.unlock();
	if (doDelete)
	{
	    delete this;
	}
    }

    int __getRef()
    {
	_mutex.lock();
	int ref = _ref;
	_mutex.unlock();
	return ref;
    }

    void __setNoDelete(bool b)
    {
	_mutex.lock();
	_noDelete = b;
	_mutex.unlock();
    }

private:

    int _ref;
    bool _noDelete;
    JTCMutex _mutex;
};

}

#endif
