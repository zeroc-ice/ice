// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UDP_CONNECTOR_H
#define ICE_UDP_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/ProtocolInstanceF.h>
#include <Ice/Connector.h>
#include <Ice/Network.h>

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

    UdpConnector(const ProtocolInstancePtr&, const Address&, const Address&, const std::string&, int,
                 const std::string&);

    virtual ~UdpConnector();
    friend class UdpEndpointI;

    const ProtocolInstancePtr _instance;
    const Address _addr;
    const Address _sourceAddr;
    const std::string _mcastInterface;
    const int _mcastTtl;
    const std::string _connectionId;
};

}
#endif
