// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OUTGOING_H
#define ICE_OUTGOING_H

#include <Ice/ReferenceF.h>

namespace _Ice
{

class ICE_API Outgoing
{
public:

    Outgoing(const Reference&, const std::string&);
    ~Outgoing();

    void invoke();

private:

    Outgoing(const Outgoing&);
    void operator=(const Outgoing&);

    Reference reference_;
    std::string operation_;
};

}

#endif
