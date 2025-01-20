// Copyright (c) ZeroC, Inc.

#ifndef ICE_STREAM_CONNECTOR_H
#define ICE_STREAM_CONNECTOR_H

#include "Ice/Config.h"

#if TARGET_OS_IPHONE != 0

#    include "../Connector.h"
#    include "../TransceiverF.h"

namespace IceObjC
{
    class StreamEndpointI;

    class Instance;
    using InstancePtr = std::shared_ptr<Instance>;

    class StreamConnector final : public IceInternal::Connector
    {
    public:
        StreamConnector(const InstancePtr&, const std::string&, std::int32_t, std::int32_t, const std::string&);
        ~StreamConnector();
        IceInternal::TransceiverPtr connect() final;

        std::int16_t type() const final;
        std::string toString() const final;

        bool operator==(const IceInternal::Connector&) const final;
        bool operator<(const IceInternal::Connector&) const final;

    private:
        friend class StreamEndpointI;

        const InstancePtr _instance;
        const std::string _host;
        const std::int32_t _port;
        const std::int32_t _timeout;
        const std::string _connectionId;
    };
}

#endif

#endif
