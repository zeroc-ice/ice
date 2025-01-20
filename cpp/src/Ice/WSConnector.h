// Copyright (c) ZeroC, Inc.

#ifndef ICE_WSCONNECTOR_I_H
#define ICE_WSCONNECTOR_I_H

#include "Connector.h"
#include "Ice/Logger.h"
#include "ProtocolInstance.h"
#include "TransceiverF.h"

namespace IceInternal
{
    class WSEndpoint;

    class WSConnector final : public Connector
    {
    public:
        WSConnector(ProtocolInstancePtr, ConnectorPtr, std::string, std::string);
        ~WSConnector() override;
        TransceiverPtr connect() final;

        [[nodiscard]] std::int16_t type() const final;
        [[nodiscard]] std::string toString() const final;

        bool operator==(const Connector&) const final;
        bool operator<(const Connector&) const final;

    private:
        const ProtocolInstancePtr _instance;
        const ConnectorPtr _delegate;
        const std::string _host;
        const std::string _resource;
    };
}

#endif
