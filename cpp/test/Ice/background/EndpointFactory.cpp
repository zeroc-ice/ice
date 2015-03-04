// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Instance.h>
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
EndpointFactory::create(const string& str, bool server) const
{
    return new EndpointI(_factory->create(str, server));
}

IceInternal::EndpointIPtr
EndpointFactory::read(IceInternal::BasicStream* s) const
{
    short type;
    s->read(type);
    assert(type == _factory->type());
    return new EndpointI(_factory->read(s));
}

void
EndpointFactory::destroy()
{
}
