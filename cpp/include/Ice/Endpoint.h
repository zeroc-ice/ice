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

//
// Regular endpoint using a byte-stream oriented protocol
//
class ICE_API EndpointI : virtual public Shared
{
public:

    EndpointI(const std::string&, int, int);
    virtual ~EndpointI();

    //
    // All  members are const, because Endpoints are immutable.
    //
    const std::string host;
    const int port;
    const int timeout;

    bool operator==(const EndpointI&) const;
    bool operator!=(const EndpointI&) const;
    bool operator<(const EndpointI&) const;

private:

    EndpointI(const EndpointI&);
    void operator=(const EndpointI&);
};

//
// Datagram endpoint using a packet oriented protocol
//
class ICE_API DgEndpointI : virtual public Shared
{
public:

    DgEndpointI(const std::string&, int);
    virtual ~DgEndpointI();

    //
    // All  members are const, because DgEndpoints are immutable.
    //
    const std::string host;
    const int port;

    bool operator==(const DgEndpointI&) const;
    bool operator!=(const DgEndpointI&) const;
    bool operator<(const DgEndpointI&) const;

private:

    DgEndpointI(const DgEndpointI&);
    void operator=(const DgEndpointI&);
};

}

#endif
