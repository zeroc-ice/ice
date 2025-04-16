// Copyright (c) ZeroC, Inc.

#ifndef TEST_API_EXPORTS
#    define TEST_API_EXPORTS
#endif

#include "PluginI.h"
#include "EndpointFactory.h"
#include "Ice/Ice.h"
#include "Ice/ProtocolPluginFacade.h"

using namespace std;

class TestPluginI final : public PluginI
{
public:
    TestPluginI(Ice::CommunicatorPtr);

    void initialize() final;
    void destroy() final;
    ConfigurationPtr getConfiguration() final;

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

TestPluginI::TestPluginI(Ice::CommunicatorPtr communicator)
    : _communicator(std::move(communicator)),
      _configuration(make_shared<Configuration>())
{
    _configuration->init();
}

void
TestPluginI::initialize()
{
    IceInternal::ProtocolPluginFacade facade{_communicator};

    for (int16_t s = 0; s < 100; ++s)
    {
        IceInternal::EndpointFactoryPtr factory = facade.getEndpointFactory(s);
        if (factory)
        {
            facade.addEndpointFactory(make_shared<EndpointFactory>(factory));
        }
    }
    for (int16_t s = 1000; s < 1010; ++s)
    {
        IceInternal::EndpointFactoryPtr factory = facade.getEndpointFactory(s);
        if (factory)
        {
            facade.addEndpointFactory(make_shared<EndpointFactory>(factory));
        }
    }
    for (int16_t s = 10000; s < 10010; ++s)
    {
        IceInternal::EndpointFactoryPtr factory = facade.getEndpointFactory(s);
        if (factory)
        {
            facade.addEndpointFactory(make_shared<EndpointFactory>(factory));
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
