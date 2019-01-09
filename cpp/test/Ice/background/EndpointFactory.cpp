// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef TEST_API_EXPORTS
#   define TEST_API_EXPORTS
#endif

#include <Ice/EndpointFactoryManager.h>

#include <EndpointFactory.h>
#include <EndpointI.h>

using namespace std;

EndpointFactory::EndpointFactory(const IceInternal::EndpointFactoryPtr& factory) :
    _factory(factory)
{
}

Ice::Short
EndpointFactory::type() const
{
    return (Ice::Short)(EndpointI::TYPE_BASE + _factory->type());
}

string
EndpointFactory::protocol() const
{
    return "test-" + _factory->protocol();
}

IceInternal::EndpointIPtr
EndpointFactory::create(vector<string>& args, bool oaEndpoint) const
{
    return ICE_MAKE_SHARED(EndpointI, _factory->create(args, oaEndpoint));
}

IceInternal::EndpointIPtr
EndpointFactory::read(Ice::InputStream* s) const
{
    short type;
    s->read(type);
    assert(type == _factory->type());

    s->startEncapsulation();
    IceInternal::EndpointIPtr endpoint = ICE_MAKE_SHARED(EndpointI, _factory->read(s));
    s->endEncapsulation();
    return endpoint;
}

void
EndpointFactory::destroy()
{
}

IceInternal::EndpointFactoryPtr
EndpointFactory::clone(const IceInternal::ProtocolInstancePtr&) const
{
    return const_cast<EndpointFactory*>(this);
}
