// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FLUSHABLE_H
#define FLUSHABLE_H

#include <IceUtil/Handle.h>
#include <IceUtil/Shared.h>

namespace IceStorm
{

//
// Flushable interface.
//
class Flushable : public virtual IceUtil::Shared
{
public:

    Flushable() { }
    ~Flushable() { }

    //
    // Flush any pending writes.
    //
    virtual void flush() = 0;

    //
    // Is this flushable object invalid?
    //
    virtual bool inactive() const = 0;

    //
    // Equality for flushable objects.
    //
    virtual bool operator==(const Flushable&) const = 0;
};

typedef IceUtil::Handle<Flushable> FlushablePtr;

} // End namespace IceStorm

#endif
