// Copyright (c) ZeroC, Inc.

#ifndef ICE_PROTOCOL_INSTANCE_H
#define ICE_PROTOCOL_INSTANCE_H

#include "ConnectorF.h"
#include "EndpointFactory.h"
#include "EndpointIF.h"
#include "IPEndpointIF.h"
#include "Ice/CommunicatorF.h"
#include "Ice/Logger.h"
#include "Ice/PropertiesF.h"
#include "Instance.h"
#include "NetworkF.h"
#include "ProtocolInstanceF.h"

namespace IceInternal
{
    class ICE_API ProtocolInstance
    {
    public:
        virtual ~ProtocolInstance();

        ProtocolInstance(const Ice::CommunicatorPtr&, std::int16_t, std::string, bool);

        [[nodiscard]] int traceLevel() const { return _traceLevel; }

        [[nodiscard]] const std::string& traceCategory() const { return _traceCategory; }

        [[nodiscard]] const Ice::LoggerPtr& logger() const;

        [[nodiscard]] const std::string& protocol() const { return _protocol; }

        [[nodiscard]] std::int16_t type() const { return _type; }

        [[nodiscard]] const Ice::PropertiesPtr& properties() const { return _properties; }

        [[nodiscard]] bool secure() const { return _secure; }

        [[nodiscard]] IceInternal::EndpointFactoryPtr getEndpointFactory(std::int16_t) const;
        BufSizeWarnInfo getBufSizeWarn(std::int16_t type);
        void setSndBufSizeWarn(std::int16_t type, int size);
        void setRcvBufSizeWarn(std::int16_t type, int size);
        [[nodiscard]] bool preferIPv6() const;
        [[nodiscard]] ProtocolSupport protocolSupport() const;
        [[nodiscard]] const std::string& defaultHost() const;
        [[nodiscard]] const Address& defaultSourceAddress() const;
        [[nodiscard]] const Ice::EncodingVersion& defaultEncoding() const;
        [[nodiscard]] NetworkProxyPtr networkProxy() const;
        [[nodiscard]] std::int32_t messageSizeMax() const;

        void resolve(
            const std::string&,
            int,
            const IPEndpointIPtr&,
            std::function<void(std::vector<ConnectorPtr>)>,
            std::function<void(std::exception_ptr)>) const;

    protected:
        ProtocolInstance(const InstancePtr&, std::int16_t, std::string, bool);
        friend class Instance;
        // Use a weak pointer to avoid circular references. The communicator owns the endpoint factory, which in
        // turn own this protocol instance.
        const std::weak_ptr<Instance> _instance;
        const int _traceLevel;
        const std::string _traceCategory;
        const Ice::PropertiesPtr _properties;
        const std::string _protocol;
        const std::int16_t _type;
        const bool _secure;
    };
}

#endif
