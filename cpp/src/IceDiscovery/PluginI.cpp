// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>

#include <IceDiscovery/PluginI.h>
#include <IceDiscovery/LocatorI.h>
#include <IceDiscovery/LookupI.h>

using namespace std;
using namespace IceDiscovery;

//
// Plugin factory function.
//
extern "C"
{

ICE_DECLSPEC_EXPORT Ice::Plugin*
createIceDiscovery(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginI(communicator);
}

}

PluginI::PluginI(const Ice::CommunicatorPtr& communicator) : _communicator(communicator)
{
    Ice::InitializationData initData;
    initData.properties = communicator->getProperties()->clone();
    initData.properties->setProperty("Ice.Default.CollocationOptimized", "0");
    Ice::PropertyDict props = initData.properties->getPropertiesForPrefix("Ice.Plugin.");
    for(Ice::PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
    {
        initData.properties->setProperty(p->first, "");
    }
    _pluginCommunicator = Ice::initialize(initData);
}

void
PluginI::initialize()
{
    Ice::PropertiesPtr properties = _pluginCommunicator->getProperties();

    bool ipv4 = properties->getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
    string address;
    if(ipv4)
    {
        address = properties->getPropertyWithDefault("IceDiscovery.Address", "239.255.0.1");
    }
    else
    {
        address = properties->getPropertyWithDefault("IceDiscovery.Address", "ff15::1");
    }
    int port = properties->getPropertyAsIntWithDefault("IceDiscovery.Port", 4061);
    string interface = properties->getProperty("IceDiscovery.Interface");

    if(properties->getProperty("IceDiscovery.Multicast.Endpoints").empty())
    {
        ostringstream os;
        os << "udp -h \"" << address << "\" -p " << port;
        if(!interface.empty())
        {
            os << " --interface \"" << interface << "\"";
        }
        properties->setProperty("IceDiscovery.Multicast.Endpoints", os.str());
    }
    if(properties->getProperty("IceDiscovery.Reply.Endpoints").empty())
    {
        ostringstream os;
        os << "udp";
        if(!interface.empty())
        {
            os << " -h \"" << interface << "\"";
        }
        properties->setProperty("IceDiscovery.Reply.Endpoints", os.str());
    }
    if(properties->getProperty("IceDiscovery.Locator.Endpoints").empty())
    {
        if(ipv4)
        {
            properties->setProperty("IceDiscovery.Locator.Endpoints", "tcp -h 127.0.0.1");
        }
        else
        {
            properties->setProperty("IceDiscovery.Locator.Endpoints", "tcp -h \"::1\"");
        }
    }

    Ice::ObjectAdapterPtr multicastAdapter = _pluginCommunicator->createObjectAdapter("IceDiscovery.Multicast");
    Ice::ObjectAdapterPtr replyAdapter = _pluginCommunicator->createObjectAdapter("IceDiscovery.Reply");
    Ice::ObjectAdapterPtr locatorAdapter = _pluginCommunicator->createObjectAdapter("IceDiscovery.Locator");

    //
    // Setup locatory registry.
    //
    LocatorRegistryIPtr locatorRegistry = new LocatorRegistryI(_communicator);
    Ice::LocatorRegistryPrx locatorRegistryPrx = 
        Ice::LocatorRegistryPrx::uncheckedCast(locatorAdapter->addWithUUID(locatorRegistry));

    string lookupEndpoints = properties->getProperty("IceDiscovery.Lookup");
    if(lookupEndpoints.empty())
    {
        ostringstream os;
        os << "udp -h \"" << address << "\" -p " << port;
        if(!interface.empty())
        {
            os << " --interface \"" << interface << "\"";
        }
        lookupEndpoints = os.str();
    }

    Ice::ObjectPrx lookupPrx = _pluginCommunicator->stringToProxy("IceDiscovery/Lookup -d:" + lookupEndpoints);
    lookupPrx = lookupPrx->ice_collocationOptimized(false);

    //
    // Add lookup and lookup reply Ice objects
    //
    LookupIPtr lookup = new LookupI(locatorRegistry, LookupPrx::uncheckedCast(lookupPrx), properties);
    multicastAdapter->add(lookup, _pluginCommunicator->stringToIdentity("IceDiscovery/Lookup"));

    Ice::ObjectPrx lookupReply = replyAdapter->addWithUUID(new LookupReplyI(lookup))->ice_datagram();
    lookup->setLookupReply(LookupReplyPrx::uncheckedCast(lookupReply));

    //
    // Setup locator on the communicator.
    //
    Ice::ObjectPrx loc = locatorAdapter->addWithUUID(new LocatorI(lookup, locatorRegistryPrx));
    _communicator->setDefaultLocator(Ice::LocatorPrx::uncheckedCast(_communicator->stringToProxy(loc->ice_toString())));
    
    multicastAdapter->activate();
    replyAdapter->activate();
    locatorAdapter->activate();
}

void
PluginI::destroy()
{
    _pluginCommunicator->destroy();
}
