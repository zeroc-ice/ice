//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_TCP_CONNECTOR_H
#define ICE_TCP_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/ProtocolInstanceF.h>
#include <Ice/Connector.h>
#include <Ice/Network.h>

namespace IceInternal
{

class TcpConnector final : public Connector
{
public:

    TcpConnector(const ProtocolInstancePtr&, const Address&, const NetworkProxyPtr&, const Address&, Ice::Int,
                 const std::string&);
    ~TcpConnector();
    TransceiverPtr connect() final;

    Ice::Short type() const final;
    std::string toString() const final;

    bool operator==(const Connector&) const final;
    bool operator<(const Connector&) const final;

private:

    const ProtocolInstancePtr _instance;
    const Address _addr;
    const NetworkProxyPtr _proxy;
    const Address _sourceAddr;
    const Ice::Int _timeout;
    const std::string _connectionId;
};

}

#endif
