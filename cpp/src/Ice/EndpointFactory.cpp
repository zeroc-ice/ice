// Copyright (c) ZeroC, Inc.

#include "EndpointFactory.h"
#include "EndpointFactoryManager.h"
#include "Instance.h"
#include "ProtocolInstance.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::EndpointFactory::EndpointFactory() = default;

IceInternal::EndpointFactory::~EndpointFactory() = default;

void
IceInternal::EndpointFactory::initialize()
{
    // Nothing to do, can be overridden by specialization to finish initialization.
}

IceInternal::EndpointFactoryPlugin::EndpointFactoryPlugin(
    const CommunicatorPtr& communicator,
    const EndpointFactoryPtr& factory)
{
    assert(communicator);
    getInstance(communicator)->endpointFactoryManager()->add(factory);
}

void
IceInternal::EndpointFactoryPlugin::initialize()
{
}

void
IceInternal::EndpointFactoryPlugin::destroy()
{
}

IceInternal::EndpointFactoryWithUnderlying::EndpointFactoryWithUnderlying(ProtocolInstancePtr instance, int16_t type)
    : _instance(std::move(instance)),
      _type(type)
{
}

void
IceInternal::EndpointFactoryWithUnderlying::initialize()
{
    //
    // Get the endpoint factory for the underlying type and clone it with
    // our protocol instance.
    //
    EndpointFactoryPtr factory = _instance->getEndpointFactory(_type);
    if (factory)
    {
        _underlying = factory->clone(_instance);
        _underlying->initialize();
    }
}

int16_t
IceInternal::EndpointFactoryWithUnderlying::type() const
{
    return _instance->type();
}

string
IceInternal::EndpointFactoryWithUnderlying::protocol() const
{
    return _instance->protocol();
}

EndpointIPtr
IceInternal::EndpointFactoryWithUnderlying::create(vector<string>& args, bool oaEndpoint) const
{
    if (!_underlying)
    {
        return nullptr; // Can't create an endpoint without underlying factory.
    }
    return createWithUnderlying(_underlying->create(args, oaEndpoint), args, oaEndpoint);
}

EndpointIPtr
IceInternal::EndpointFactoryWithUnderlying::read(InputStream* s) const
{
    if (!_underlying)
    {
        return nullptr; // Can't create an endpoint without underlying factory.
    }
    return readWithUnderlying(_underlying->read(s), s);
}

EndpointFactoryPtr
IceInternal::EndpointFactoryWithUnderlying::clone(const ProtocolInstancePtr& instance) const
{
    return cloneWithUnderlying(instance, _type);
}

IceInternal::UnderlyingEndpointFactory::UnderlyingEndpointFactory(
    ProtocolInstancePtr instance,
    int16_t type,
    int16_t underlying)
    : _instance(std::move(instance)),
      _type(type),
      _underlying(underlying)
{
}

void
IceInternal::UnderlyingEndpointFactory::initialize()
{
    //
    // Get the endpoint factory of the given endpoint type. If it's a factory that
    // delegates to an underlying endpoint, clone it and instruct it to delegate to
    // our underlying factory.
    //
    EndpointFactoryPtr factory = _instance->getEndpointFactory(_type);
    if (factory)
    {
        auto f = dynamic_pointer_cast<EndpointFactoryWithUnderlying>(factory);
        if (f)
        {
            _factory = f->cloneWithUnderlying(_instance, _underlying);
            _factory->initialize();
        }
    }
}

int16_t
IceInternal::UnderlyingEndpointFactory::type() const
{
    return _instance->type();
}

string
IceInternal::UnderlyingEndpointFactory::protocol() const
{
    return _instance->protocol();
}

EndpointIPtr
IceInternal::UnderlyingEndpointFactory::create(vector<string>& args, bool oaEndpoint) const
{
    if (!_factory)
    {
        return nullptr;
    }
    return _factory->create(args, oaEndpoint);
}

EndpointIPtr
IceInternal::UnderlyingEndpointFactory::read(InputStream* s) const
{
    if (!_factory)
    {
        return nullptr;
    }
    return _factory->read(s);
}

EndpointFactoryPtr
IceInternal::UnderlyingEndpointFactory::clone(const ProtocolInstancePtr& instance) const
{
    return make_shared<UnderlyingEndpointFactory>(instance, _type, _underlying);
}
