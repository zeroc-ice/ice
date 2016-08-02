// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

#ifndef ICE_DISCOVERY_API
#   ifdef ICE_DISCOVERY_API_EXPORTS
#       define ICE_DISCOVERY_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_DISCOVERY_API /**/
#   endif
#endif

//
// Plugin factory function.
//
extern "C" ICE_DISCOVERY_API Ice::Plugin*
createIceDiscovery(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new PluginI(communicator);
}

namespace Ice
{

ICE_DISCOVERY_API void
registerIceDiscovery(bool loadOnInitialize)
{
    Ice::registerPluginFactory("IceDiscovery", createIceDiscovery, loadOnInitialize);
}

}

//
// Objective-C function to allow Objective-C programs to register plugin.
//
extern "C" ICE_DISCOVERY_API void
ICEregisterIceDiscovery(bool loadOnInitialize)
{
    Ice::registerIceDiscovery(loadOnInitialize);
}

PluginI::PluginI(const Ice::CommunicatorPtr& communicator) : _communicator(communicator)
{
}

void
PluginI::initialize()
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

    bool ipv4 = properties->getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
    bool preferIPv6 = properties->getPropertyAsInt("Ice.PreferIPv6Address") > 0;
    string address;
    if(ipv4 && !preferIPv6)
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
        properties->setProperty("IceDiscovery.Locator.AdapterId", Ice::generateUUID());
    }

    _multicastAdapter = _communicator->createObjectAdapter("IceDiscovery.Multicast");
    _replyAdapter = _communicator->createObjectAdapter("IceDiscovery.Reply");
    _locatorAdapter = _communicator->createObjectAdapter("IceDiscovery.Locator");

    //
    // Setup locatory registry.
    //
    LocatorRegistryIPtr locatorRegistry = ICE_MAKE_SHARED(LocatorRegistryI, _communicator);
    Ice::LocatorRegistryPrxPtr locatorRegistryPrx =
        ICE_UNCHECKED_CAST(Ice::LocatorRegistryPrx, _locatorAdapter->addWithUUID(locatorRegistry));

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

    Ice::ObjectPrxPtr lookupPrx = _communicator->stringToProxy("IceDiscovery/Lookup -d:" + lookupEndpoints);
    lookupPrx = lookupPrx->ice_collocationOptimized(false); // No collocation optimization for the multicast proxy!
    try
    {
        // Ensure we can establish a connection to the multicast proxy
        // but don't block.
#ifdef ICE_CPP11_MAPPING
        lookupPrx->ice_getConnection();
#else
        Ice::AsyncResultPtr result = lookupPrx->begin_ice_getConnection();
        if(result->sentSynchronously())
        {
            lookupPrx->end_ice_getConnection(result);
        }
#endif
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << "IceDiscovery is unable to establish a multicast connection:\n";
        os << "proxy = " << lookupPrx << '\n';
        os << ex;
        throw Ice::PluginInitializationException(__FILE__, __LINE__, os.str());
    }

    //
    // Add lookup and lookup reply Ice objects
    //
    _lookup = ICE_MAKE_SHARED(LookupI, locatorRegistry, ICE_UNCHECKED_CAST(LookupPrx, lookupPrx), properties);
    _multicastAdapter->add(_lookup, Ice::stringToIdentity("IceDiscovery/Lookup"));

    Ice::ObjectPrxPtr lookupReply = _replyAdapter->addWithUUID(ICE_MAKE_SHARED(LookupReplyI, _lookup))->ice_datagram();
    _lookup->setLookupReply(ICE_UNCHECKED_CAST(LookupReplyPrx, lookupReply));

    //
    // Setup locator on the communicator.
    //
    Ice::ObjectPrxPtr loc = _locatorAdapter->addWithUUID(ICE_MAKE_SHARED(LocatorI, _lookup, locatorRegistryPrx));
    _communicator->setDefaultLocator(ICE_UNCHECKED_CAST(Ice::LocatorPrx, loc));

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
    _lookup->destroy();
}
