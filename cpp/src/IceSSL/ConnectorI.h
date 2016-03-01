// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTOR_I_H
#define ICE_SSL_CONNECTOR_I_H

#include <Ice/TransceiverF.h>
#include <Ice/Connector.h>
#include <Ice/Network.h>

#include <IceSSL/InstanceF.h>

namespace IceSSL
{

class EndpointI;

class ConnectorI : public IceInternal::Connector
{
public:

    virtual IceInternal::TransceiverPtr connect();

    virtual Ice::Short type() const;
    virtual std::string toString() const;

    virtual bool operator==(const IceInternal::Connector&) const;
    virtual bool operator!=(const IceInternal::Connector&) const;
    virtual bool operator<(const IceInternal::Connector&) const;

private:

    ConnectorI(const InstancePtr&, const std::string&, const IceInternal::Address&,
               const IceInternal::NetworkProxyPtr&, const IceInternal::Address&,
               Ice::Int, const std::string&);
    virtual ~ConnectorI();
    friend class EndpointI;

    const InstancePtr _instance;
    const std::string _host;
    const IceInternal::Address _addr;
    const IceInternal::NetworkProxyPtr _proxy;
    const IceInternal::Address _sourceAddr;
    const Ice::Int _timeout;
    const std::string _connectionId;
};

}

#endif
