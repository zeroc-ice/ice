// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UWP_TCP_CONNECTOR_H
#define ICE_UWP_TCP_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/ProtocolInstanceF.h>
#include <Ice/Connector.h>
#include <Ice/Network.h>

namespace IceInternal
{

class TcpConnector : public Connector
{
public:

    virtual TransceiverPtr connect();

    virtual Ice::Short type() const;
    virtual std::string toString() const;

    virtual bool operator==(const Connector&) const;
    virtual bool operator!=(const Connector&) const;
    virtual bool operator<(const Connector&) const;

private:

    TcpConnector(const ProtocolInstancePtr&, const Address&, Ice::Int, const std::string&);
    friend class TcpEndpointI;

    const ProtocolInstancePtr _instance;
    const Address _addr;
    const Ice::Int _timeout;
    const std::string _connectionId;
};

}

#endif
