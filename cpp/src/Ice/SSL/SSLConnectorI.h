// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_CONNECTOR_I_H
#define ICE_SSL_CONNECTOR_I_H

#include "../Connector.h"
#include "../Network.h"
#include "../TransceiverF.h"
#include "SSLInstanceF.h"

namespace Ice::SSL
{
    class EndpointI;

    class ConnectorI final : public IceInternal::Connector
    {
    public:
        ConnectorI(InstancePtr, IceInternal::ConnectorPtr, std::string);
        ~ConnectorI() override;
        IceInternal::TransceiverPtr connect() final;

        [[nodiscard]] std::int16_t type() const final;
        [[nodiscard]] std::string toString() const final;

        bool operator==(const IceInternal::Connector&) const final;
        bool operator<(const IceInternal::Connector&) const final;

    private:
        friend class EndpointI;

        const InstancePtr _instance;
        const IceInternal::ConnectorPtr _delegate;
        const std::string _host;
    };

}

#endif
