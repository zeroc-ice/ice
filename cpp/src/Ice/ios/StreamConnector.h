// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_STREAM_CONNECTOR_H
#define ICE_STREAM_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/Connector.h>

namespace IceObjC
{

class StreamEndpointI;

class Instance;
typedef IceUtil::Handle<Instance> InstancePtr;

class StreamConnector : public IceInternal::Connector
{
public:

    virtual IceInternal::TransceiverPtr connect();

    virtual Ice::Short type() const;
    virtual std::string toString() const;

    virtual bool operator==(const IceInternal::Connector&) const;
    virtual bool operator<(const IceInternal::Connector&) const;

private:

    StreamConnector(const InstancePtr&, const std::string&, Ice::Int, Ice::Int, const std::string&);
    virtual ~StreamConnector();
    friend class StreamEndpointI;

    const InstancePtr _instance;
    const std::string _host;
    const Ice::Int _port;
    const Ice::Int _timeout;
    const std::string _connectionId;
};

}

#endif
