// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UDP_CONNECTOR_H
#define ICE_UDP_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/InstanceF.h>
#include <Ice/Connector.h>

#ifdef _WIN32
#   include <winsock2.h>
#else
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceInternal
{

class UdpConnector : public Connector
{
public:
    
    virtual TransceiverPtr connect(int);
    virtual Ice::Short type() const;
    virtual std::string toString() const;

    virtual bool operator==(const Connector&) const;
    virtual bool operator!=(const Connector&) const;
    virtual bool operator<(const Connector&) const;
    
private:
    
    UdpConnector(const InstancePtr&, const struct sockaddr_in&, const std::string&, int, Ice::Byte, Ice::Byte, 
                 Ice::Byte, Ice::Byte, const std::string&);
    virtual ~UdpConnector();
    friend class UdpEndpointI;

    const InstancePtr _instance;
    struct sockaddr_in _addr;
    const std::string _mcastInterface;
    const int _mcastTtl;
    const Ice::Byte _protocolMajor;
    const Ice::Byte _protocolMinor;
    const Ice::Byte _encodingMajor;
    const Ice::Byte _encodingMinor;
    const std::string _connectionId;
};

}

#endif
