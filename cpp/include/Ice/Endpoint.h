// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ENDPOINT_H
#define ICE_ENDPOINT_H

#include <Ice/EndpointF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class ICE_API TcpEndpointI : public Shared
{
public:

    TcpEndpointI(const std::string&, int, int);

    //
    // All  members are const, because TcpEndpoints are immutable.
    //
    const std::string host;
    const int port;
    const int timeout;

    bool operator==(const TcpEndpointI&) const;
    bool operator!=(const TcpEndpointI&) const;
    bool operator<(const TcpEndpointI&) const;

private:

    TcpEndpointI(const TcpEndpointI&);
    void operator=(const TcpEndpointI&);
};

class ICE_API UdpEndpointI : public Shared
{
public:

    UdpEndpointI(const std::string&, int);

    //
    // All  members are const, because UdpEndpoints are immutable.
    //
    const std::string host;
    const int port;

    bool operator==(const UdpEndpointI&) const;
    bool operator!=(const UdpEndpointI&) const;
    bool operator<(const UdpEndpointI&) const;

private:

    UdpEndpointI(const UdpEndpointI&);
    void operator=(const UdpEndpointI&);
};

}

#endif
