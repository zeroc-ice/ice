// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BT_CONNECTOR_I_H
#define ICE_BT_CONNECTOR_I_H

#include <IceBT/Config.h>
#include <IceBT/Engine.h>
#include <IceBT/InstanceF.h>

#include <Ice/TransceiverF.h>
#include <Ice/Connector.h>

namespace IceBT
{

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

    ConnectorI(const InstancePtr&, const std::string&, const std::string&, Ice::Int, const std::string&);
    friend class EndpointI;

    const InstancePtr _instance;
    const std::string _addr;
    const std::string _uuid;
    const Ice::Int _timeout;
    const std::string _connectionId;
};

}

#endif
