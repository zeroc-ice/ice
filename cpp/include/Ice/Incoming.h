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
#include <Ice/Stream.h>

namespace _Ice
{

class ICE_API Incoming
{
public:

    Incoming(const Collector&);
    ~Incoming();

    Stream* os() { return &os_; }
    Stream* is() { return &is_; }
    const std::string& operation() const { return operation_; }

private:

    Incoming(const Incoming&);
    void operator=(const Incoming&);

    const Collector& collector_;
    std::string operation_;
    Stream os_;
    Stream is_;
};

}

#endif
