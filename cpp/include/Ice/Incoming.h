// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_INCOMING_H
#define ICE_INCOMING_H

#include <Ice/CollectorF.h>
#include <Ice/Buffer.h>

namespace _Ice
{

class ICE_API Incoming
{
public:

    Incoming(const Collector&);
    ~Incoming();

    Buffer* obuf() { return &obuf_; }
    Buffer* ibuf() { return &ibuf_; }

private:

    Incoming(const Incoming&);
    void operator=(const Incoming&);

    const Collector& collector_;
    const char* operation_;
    Buffer obuf_;
    Buffer ibuf_;
};

}

#endif
