//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_CONNECTOR_I_H
#define ICESSL_CONNECTOR_I_H

#include "Ice/TransceiverF.h"
#include "Ice/Connector.h"
#include "Ice/Network.h"

#include <IceSSL/InstanceF.h>

namespace IceSSL
{
    class EndpointI;

    class ConnectorI final : public IceInternal::Connector
    {
    public:
        ConnectorI(const InstancePtr&, const IceInternal::ConnectorPtr&, const std::string&);
        ~ConnectorI();
        IceInternal::TransceiverPtr connect() final;

        std::int16_t type() const final;
        std::string toString() const final;

        bool operator==(const IceInternal::Connector&) const final;
        bool operator<(const IceInternal::Connector&) const final;

    private:
        friend class EndpointI;

        const InstancePtr _instance;
        const IceInternal::ConnectorPtr _delegate;
        const std::string _host;
    };

} // IceSSL namespace end

#endif
