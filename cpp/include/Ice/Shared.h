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
#include <cassert>

namespace Ice
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

    void _incRef()
    {
	++ref_;
    }

    void _decRef()
    {
	assert(ref_ > 0);
	if(--ref_ == 0)
	    delete this;
    }

private:

    SimpleShared(const SimpleShared&);
    void operator=(const SimpleShared&);

    int ref_; // The reference count
};

}

#endif
