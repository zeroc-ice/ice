// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ENDPOINT_DATA_H
#define ICE_ENDPOINT_DATA_H

#include <Ice/EndpointDataF.h>
#include <Ice/Shared.h>

namespace _Ice
{

class ICE_API EndpointDataI : virtual public Shared
{
public:

    EndpointDataI(const std::string&, int, bool = false);
    virtual ~EndpointDataI();

    bool operator==(const EndpointDataI&) const;
    bool operator!=(const EndpointDataI&) const;
    bool operator<(const EndpointDataI&) const;

    //
    // All data members are const, because EndpointDatas are immutable.
    //
    const std::string host;
    const int port;
    const bool datagram;

private:

    EndpointDataI(const EndpointDataI&);
    void operator=(const EndpointDataI&);
};

}

#endif
