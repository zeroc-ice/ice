// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_REFERENCE_H
#define ICE_REFERENCE_H

#include <Ice/ReferenceF.h>
#include <Ice/InstanceF.h>
#include <Ice/Shared.h>

namespace _Ice
{

class ICE_API ReferenceI : virtual public SimpleShared
{
public:

    ReferenceI(const Instance&, const std::string&, int, const std::string&,
	       bool = false);
    virtual ~ReferenceI();

    bool operator==(const ReferenceI&) const;
    bool operator!=(const ReferenceI&) const;

    //
    // All data members are const, because References are immutable.
    //
    const Instance instance;
    const std::string host;
    const int port;
    const std::string identity;
    const bool datagram;

private:

    ReferenceI(const ReferenceI&);
    void operator=(const ReferenceI&);
};

}

#endif
