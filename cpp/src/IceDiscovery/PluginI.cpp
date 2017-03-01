// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Ice/Network.h> // For getInterfacesForMulticast

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
    string intf = properties->getProperty("IceDiscovery.Interface");

    if(properties->getProperty("IceDiscovery.Multicast.Endpoints").empty())
    {
        ostringstream os;
        os << "udp -h \"" << address << "\" -p " << port;
        if(!intf.empty())
        {
            os << " --interface \"" << intf << "\"";
        }
        properties->setProperty("IceDiscovery.Multicast.Endpoints", os.str());
    }

    string lookupEndpoints = properties->getProperty("IceDiscovery.Lookup");
    if(lookupEndpoints.empty())
    {
        //
        // If no lookup endpoints are specified, we get all the network interfaces and create
        // an endpoint for each of them. We'll send UDP multicast packages on each interface.
        //
        IceInternal::ProtocolSupport protocol = ipv4 && !preferIPv6 ? IceInternal::EnableIPv4 : IceInternal::EnableIPv6;
        vector<string> interfaces = IceInternal::getInterfacesForMulticast(intf, protocol);
        ostringstream lookup;
        for(vector<string>::const_iterator p = interfaces.begin(); p != interfaces.end(); ++p)
        {
            if(p != interfaces.begin())
            {
                lookup << ":";
            }
            lookup << "udp -h \"" << address << "\" -p " << port << " --interface \"" << *p << "\"";
        }
        lookupEndpoints = lookup.str();
    }

    if(properties->getProperty("IceDiscovery.Reply.Endpoints").empty())
    {
        properties->setProperty("IceDiscovery.Reply.Endpoints", "udp -h " + (intf.empty() ? "*" : "\"" + intf + "\""));
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

    Ice::ObjectPrxPtr lookupPrx = _communicator->stringToProxy("IceDiscovery/Lookup -d:" + lookupEndpoints);
    lookupPrx = lookupPrx->ice_collocationOptimized(false); // No collocation optimization for the multicast proxy!

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
