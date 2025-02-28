// Copyright (c) ZeroC, Inc.

#include "ProtocolInstance.h"
#include "DefaultsAndOverrides.h"
#include "EndpointFactoryManager.h"
#include "IPEndpointI.h"
#include "Ice/Initialize.h"
#include "Instance.h"
#include "TraceLevels.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

// Out of line to avoid weak vtable
IceInternal::ProtocolInstance::~ProtocolInstance() = default;

IceInternal::ProtocolInstance::ProtocolInstance(const CommunicatorPtr& com, int16_t type, string protocol, bool secure)
    : _instance(getInstance(com)),
      _traceLevel(_instance.lock()->traceLevels()->network),
      _traceCategory(_instance.lock()->traceLevels()->networkCat),
      _properties(_instance.lock()->initializationData().properties),
      _protocol(std::move(protocol)),
      _type(type),
      _secure(secure)
{
}

IceInternal::ProtocolInstance::ProtocolInstance(const InstancePtr& instance, int16_t type, string protocol, bool secure)
    : _instance(instance),
      _traceLevel(instance->traceLevels()->network),
      _traceCategory(instance->traceLevels()->networkCat),
      _properties(instance->initializationData().properties),
      _protocol(std::move(protocol)),
      _type(type),
      _secure(secure)
{
}

const LoggerPtr&
IceInternal::ProtocolInstance::logger() const
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return instance->initializationData().logger;
}

EndpointFactoryPtr
IceInternal::ProtocolInstance::getEndpointFactory(int16_t type) const
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return instance->endpointFactoryManager()->get(type);
}

BufSizeWarnInfo
IceInternal::ProtocolInstance::getBufSizeWarn(int16_t type)
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return instance->getBufSizeWarn(type);
}

void
IceInternal::ProtocolInstance::setSndBufSizeWarn(int16_t type, int size)
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    instance->setSndBufSizeWarn(type, size);
}

void
IceInternal::ProtocolInstance::setRcvBufSizeWarn(int16_t type, int size)
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    instance->setRcvBufSizeWarn(type, size);
}

bool
IceInternal::ProtocolInstance::preferIPv6() const
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return instance->preferIPv6();
}

ProtocolSupport
IceInternal::ProtocolInstance::protocolSupport() const
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return instance->protocolSupport();
}

const string&
IceInternal::ProtocolInstance::defaultHost() const
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return instance->defaultsAndOverrides()->defaultHost;
}

const Address&
IceInternal::ProtocolInstance::defaultSourceAddress() const
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return instance->defaultsAndOverrides()->defaultSourceAddress;
}

const EncodingVersion&
IceInternal::ProtocolInstance::defaultEncoding() const
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return instance->defaultsAndOverrides()->defaultEncoding;
}

NetworkProxyPtr
IceInternal::ProtocolInstance::networkProxy() const
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return instance->networkProxy();
}

size_t
IceInternal::ProtocolInstance::messageSizeMax() const
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return instance->messageSizeMax();
}

void
IceInternal::ProtocolInstance::resolve(
    const string& host,
    int port,
    const IPEndpointIPtr& endpoint,
    std::function<void(std::vector<ConnectorPtr>)> response,
    std::function<void(exception_ptr)> exception) const
{
    InstancePtr instance = _instance.lock();
    if (!instance)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    instance->endpointHostResolver()->resolve(host, port, endpoint, std::move(response), std::move(exception));
}
