//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_STREAM_CONNECTOR_H
#define ICE_STREAM_CONNECTOR_H

#include <Ice/Config.h>

#if TARGET_OS_IPHONE != 0

#include <Ice/TransceiverF.h>
#include <Ice/Connector.h>

namespace IceObjC
{

class StreamEndpointI;

class Instance;
using InstancePtr = std::shared_ptr<Instance>;

class StreamConnector final : public IceInternal::Connector
{
public:

    StreamConnector(const InstancePtr&, const std::string&, Ice::Int, Ice::Int, const std::string&);
    ~StreamConnector();
    IceInternal::TransceiverPtr connect() final;

    Ice::Short type() const final;
    std::string toString() const final;

    bool operator==(const IceInternal::Connector&) const final;
    bool operator<(const IceInternal::Connector&) const final;

private:

    friend class StreamEndpointI;

    const InstancePtr _instance;
    const std::string _host;
    const Ice::Int _port;
    const Ice::Int _timeout;
    const std::string _connectionId;
};

}

#endif

#endif
