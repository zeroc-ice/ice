// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_UWP_CONNECTOR_I_H
#define ICE_SSL_UWP_CONNECTOR_I_H

#include <Ice/TransceiverF.h>
#include <Ice/ProtocolInstanceF.h>
#include <Ice/Connector.h>
#include <Ice/Network.h>

namespace IceSSL
{

class ConnectorI : public IceInternal::Connector
{
public:

    virtual IceInternal::TransceiverPtr connect();

    virtual Ice::Short type() const;
    virtual std::string toString() const;

    virtual bool operator==(const IceInternal::Connector&) const;
    virtual bool operator<(const IceInternal::Connector&) const;

private:

    ConnectorI(const IceInternal::ProtocolInstancePtr&, const IceInternal::Address&, Ice::Int, const std::string&);
    friend class EndpointI;

    const IceInternal::ProtocolInstancePtr _instance;
    const IceInternal::Address _addr;
    const Ice::Int _timeout;
    const std::string _connectionId;
};

}

#endif
