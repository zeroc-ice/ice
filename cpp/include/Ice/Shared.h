// **********************************************************************
//
// Copyright (c) 2002
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
class ICE_API SimpleShared
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
	assert(ref_ > 0);
	if(--ref_ == 0)
	    delete this;
    }

private:

    SimpleShared(const SimpleShared&);
    void operator=(const SimpleShared&);

    int ref_;
};

//
// Thread-safe intrusive base class for referenceData-counted types.
//
class ICE_API Shared
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
	assert(ref_ > 0);
	bool doDelete = false;
	if(--ref_ == 0)
	    doDelete = true;
	mutex_.unlock();
	if(doDelete)
	    delete this;
    }

private:

    Shared(const Shared&);
    void operator=(const Shared&);

    int ref_;
    JTCMutex mutex_;
};

}

#endif
