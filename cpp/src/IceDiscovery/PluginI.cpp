// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
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
}

void
PluginI::initialize()
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

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
        properties->setProperty("IceDiscovery.Locator.AdapterId", IceUtil::generateUUID());
    }

    _multicastAdapter = _communicator->createObjectAdapter("IceDiscovery.Multicast");
    _replyAdapter = _communicator->createObjectAdapter("IceDiscovery.Reply");
    _locatorAdapter = _communicator->createObjectAdapter("IceDiscovery.Locator");

    //
    // Setup locatory registry.
    //
    LocatorRegistryIPtr locatorRegistry = new LocatorRegistryI(_communicator);
    Ice::LocatorRegistryPrx locatorRegistryPrx = 
        Ice::LocatorRegistryPrx::uncheckedCast(_locatorAdapter->addWithUUID(locatorRegistry));

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

    Ice::ObjectPrx lookupPrx = _communicator->stringToProxy("IceDiscovery/Lookup -d:" + lookupEndpoints);
    lookupPrx = lookupPrx->ice_collocationOptimized(false); // No collocation optimization for the multicast proxy!
    try
    {
        // Ensure we can establish a connection to the multicast proxy
        lookupPrx->ice_getConnection();
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << "unable to establish multicast connection, IceDiscovery will be disabled:\n";
        os << "proxy = " << lookupPrx << '\n';
        os << ex;
        throw Ice::PluginInitializationException(__FILE__, __LINE__, os.str());
    }

    //
    // Add lookup and lookup reply Ice objects
    //
    LookupIPtr lookup = new LookupI(locatorRegistry, LookupPrx::uncheckedCast(lookupPrx), properties);
    _multicastAdapter->add(lookup, _communicator->stringToIdentity("IceDiscovery/Lookup"));

    Ice::ObjectPrx lookupReply = _replyAdapter->addWithUUID(new LookupReplyI(lookup))->ice_datagram();
    lookup->setLookupReply(LookupReplyPrx::uncheckedCast(lookupReply));

    //
    // Setup locator on the communicator.
    //
    Ice::ObjectPrx loc = _locatorAdapter->addWithUUID(new LocatorI(lookup, locatorRegistryPrx));
    _communicator->setDefaultLocator(Ice::LocatorPrx::uncheckedCast(_communicator->stringToProxy(loc->ice_toString())));
    
    _multicastAdapter->activate();
    _replyAdapter->activate();
    _locatorAdapter->activate();
}

void
PluginI::destroy()
{
    _multicastAdapter->destroy();
    _replyAdapter->destroy();
    _locatorAdapter->destroy();
}
