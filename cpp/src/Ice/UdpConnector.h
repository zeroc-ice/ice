// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UDP_CONNECTOR_H
#define ICE_UDP_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/EndpointF.h>
#include <Ice/InstanceF.h>
#include <Ice/Connector.h>
#include <Ice/Protocol.h>

#ifdef _WIN32
#   include <winsock2.h>
#else
#   include <sys/socket.h> // For struct sockaddr_storage
#endif

namespace IceInternal
{

class UdpConnector : public Connector
{
public:
    
    virtual TransceiverPtr connect();

    virtual Ice::Short type() const;
    virtual std::string toString() const;

    virtual bool operator==(const Connector&) const;
    virtual bool operator!=(const Connector&) const;
    virtual bool operator<(const Connector&) const;

private:
    
    UdpConnector(const InstancePtr&, const Ice::UdpEndpointInfoPtr&, const struct sockaddr_storage&,
                 const std::string&);
    virtual ~UdpConnector();
    friend class UdpEndpointI;

    const InstancePtr _instance;
    const Ice::UdpEndpointInfoPtr _endpointInfo;
    struct sockaddr_storage _addr;
    const std::string _connectionId;
};

}

#endif
