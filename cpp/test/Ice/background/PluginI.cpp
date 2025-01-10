//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_API_EXPORTS
#    define TEST_API_EXPORTS
#endif

#include "Ice/EndpointFactoryManager.h"
#include "Ice/Initialize.h"
#include "Ice/ProtocolPluginFacade.h"

#include "EndpointFactory.h"
#include "PluginI.h"

#include <utility>

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
    IceInternal::ProtocolPluginFacadePtr facade = IceInternal::getProtocolPluginFacade(_communicator);

    for (int16_t s = 0; s < 100; ++s)
    {
        IceInternal::EndpointFactoryPtr factory = facade->getEndpointFactory(s);
        if (factory)
        {
            facade->addEndpointFactory(make_shared<EndpointFactory>(factory));
        }
    }
    for (int16_t s = 1000; s < 1010; ++s)
    {
        IceInternal::EndpointFactoryPtr factory = facade->getEndpointFactory(s);
        if (factory)
        {
            facade->addEndpointFactory(make_shared<EndpointFactory>(factory));
        }
    }
    for (int16_t s = 10000; s < 10010; ++s)
    {
        IceInternal::EndpointFactoryPtr factory = facade->getEndpointFactory(s);
        if (factory)
        {
            facade->addEndpointFactory(make_shared<EndpointFactory>(factory));
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
