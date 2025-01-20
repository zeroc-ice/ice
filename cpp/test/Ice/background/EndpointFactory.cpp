// Copyright (c) ZeroC, Inc.

#ifndef TEST_API_EXPORTS
#    define TEST_API_EXPORTS
#endif

#include "EndpointFactory.h"
#include "EndpointI.h"

using namespace std;

EndpointFactory::EndpointFactory(IceInternal::EndpointFactoryPtr factory) : _factory(std::move(factory)) {}

int16_t
EndpointFactory::type() const
{
    return (int16_t)(EndpointI::TYPE_BASE + _factory->type());
}

string
EndpointFactory::protocol() const
{
    return "test-" + _factory->protocol();
}

IceInternal::EndpointIPtr
EndpointFactory::create(vector<string>& args, bool oaEndpoint) const
{
    return make_shared<EndpointI>(_factory->create(args, oaEndpoint));
}

IceInternal::EndpointIPtr
EndpointFactory::read(Ice::InputStream* s) const
{
    short type;
    s->read(type);
    assert(type == _factory->type());

    s->startEncapsulation();
    IceInternal::EndpointIPtr endpoint = make_shared<EndpointI>(_factory->read(s));
    s->endEncapsulation();
    return endpoint;
}

IceInternal::EndpointFactoryPtr
EndpointFactory::clone(const IceInternal::ProtocolInstancePtr&) const
{
    return const_cast<EndpointFactory*>(this)->shared_from_this();
}
