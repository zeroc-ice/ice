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

//
// Base classes for reference counted types. Note that the Ice
// namespace is used instead of IceInternal, so that user code can use
// these base classes as well.
//
// For "smart pointers" or "handles" for reference counted types
// derived from these base classes, use IceInternal::Handle (only for
// Ice internal stuff, of course), or Ice::Handle (for application
// code).
//
// Ice::SimpleShared
// =================
//
// A non thread-safe base class for reference-counted types.
//
// Ice::Shared
// ===========
//
// A thread-safe base class for reference-counted types.
//
namespace Ice
{

class SimpleShared : public noncopyable
{
public:

    SimpleShared() :
	_ref(0),
	_noDelete(false)
    {
    }

    virtual ~SimpleShared()
    {
    }

    void __incRef()
    {
	assert(_ref >= 0);
	++_ref;
    }

    void __decRef()
    {
	assert(_ref > 0);
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

class Shared : public noncopyable
{
public:

    Shared() :
	_ref(0),
	_noDelete(false)
    {
    }
    
    virtual ~Shared()
    {
    }

    void __incRef()
    {
	_mutex.lock();
	assert(_ref >= 0);
	++_ref;
	_mutex.unlock();
    }

    void __decRef()
    {
	_mutex.lock();
	bool doDelete = false;
	assert(_ref > 0);
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
