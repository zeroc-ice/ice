// **********************************************************************
//
// Copyright (c) 2002
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
