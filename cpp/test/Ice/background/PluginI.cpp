// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_API_EXPORTS
#   define TEST_API_EXPORTS
#endif

#include <Ice/Initialize.h>
#include <Ice/ProtocolPluginFacade.h>
#include <Ice/EndpointFactoryManager.h>

#include <PluginI.h>
#include <EndpointFactory.h>

using namespace std;

class TestPluginI : public PluginI
{
public:

    TestPluginI(const Ice::CommunicatorPtr&);

    virtual void initialize();
    virtual void destroy();

    virtual ConfigurationPtr getConfiguration();

private:

    const Ice::CommunicatorPtr _communicator;
    const ConfigurationPtr _configuration;
};

//
// Plugin factory function.
//
extern "C"
{

ICE_DECLSPEC_EXPORT Ice::Plugin*
createTestTransport(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new TestPluginI(communicator);
}

}

TestPluginI::TestPluginI(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator),
    _configuration(new Configuration())
{
}

void
TestPluginI::initialize()
{
    IceInternal::ProtocolPluginFacadePtr facade = IceInternal::getProtocolPluginFacade(_communicator);

    for(Ice::Short s = 0; s < 100; ++s)
    {
        IceInternal::EndpointFactoryPtr factory = facade->getEndpointFactory(s);
        if(factory)
        {
            facade->addEndpointFactory(new EndpointFactory(factory));
        }
    }
}

void
TestPluginI::destroy()
{
}

ConfigurationPtr
TestPluginI::getConfiguration()
{
    return _configuration;
}
