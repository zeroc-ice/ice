// Copyright (c) ZeroC, Inc.

#include "PluginI.h"
#include "../Ice/Network.h" // For getInterfacesForMulticast
#include "IceDiscovery/IceDiscovery.h"
#include "LocatorI.h"
#include "LookupI.h"

using namespace std;
using namespace IceDiscovery;

namespace
{
    const char* const discoveryPluginName = "IceDiscovery";
}

//
// Plugin factory function.
//
extern "C" ICE_DISCOVERY_API Ice::Plugin*
createIceDiscovery(const Ice::CommunicatorPtr& communicator, const string& name, const Ice::StringSeq&)
{
    string pluginName{discoveryPluginName};

    if (name != pluginName)
    {
#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4297) // function assumed not to throw an exception but does
#endif
        throw Ice::PluginInitializationException{
            __FILE__,
            __LINE__,
            "the Discovery plug-in must be named '" + pluginName + "'"};
#ifdef _MSC_VER
#    pragma warning(pop)
#endif
    }

    return new PluginI(communicator);
}

Ice::PluginFactory
IceDiscovery::discoveryPluginFactory()
{
    return {discoveryPluginName, createIceDiscovery};
}

PluginI::PluginI(Ice::CommunicatorPtr communicator) : _communicator(std::move(communicator)) {}

void
PluginI::initialize()
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

    bool ipv4 = properties->getIcePropertyAsInt("Ice.IPv4") > 0;
    bool preferIPv6 = properties->getIcePropertyAsInt("Ice.PreferIPv6Address") > 0;
    string address = properties->getIceProperty("IceDiscovery.Address");
    if (address.empty())
    {
        address = ipv4 && !preferIPv6 ? "239.255.0.1" : "ff15::1";
    }
    int port = properties->getIcePropertyAsInt("IceDiscovery.Port");
    string intf = properties->getIceProperty("IceDiscovery.Interface");

    if (properties->getIceProperty("IceDiscovery.Multicast.Endpoints").empty())
    {
        ostringstream os;
        os << "udp -h \"" << address << "\" -p " << port;
        if (!intf.empty())
        {
            os << " --interface \"" << intf << "\"";
        }
        properties->setProperty("IceDiscovery.Multicast.Endpoints", os.str());
    }

    string lookupEndpoints = properties->getIceProperty("IceDiscovery.Lookup");
    if (lookupEndpoints.empty())
    {
        //
        // If no lookup endpoints are specified, we get all the network interfaces and create
        // an endpoint for each of them. We'll send UDP multicast packages on each interface.
        //
        IceInternal::ProtocolSupport protocol = ipv4 && !preferIPv6 ? IceInternal::EnableIPv4 : IceInternal::EnableIPv6;
        vector<string> interfaces = IceInternal::getInterfacesForMulticast(intf, protocol);
        ostringstream lookup;
        for (auto p = interfaces.begin(); p != interfaces.end(); ++p)
        {
            if (p != interfaces.begin())
            {
                lookup << ":";
            }
            lookup << "udp -h \"" << address << "\" -p " << port << " --interface \"" << *p << "\"";
        }
        lookupEndpoints = lookup.str();
    }

    if (properties->getIceProperty("IceDiscovery.Reply.Endpoints").empty())
    {
        properties->setProperty("IceDiscovery.Reply.Endpoints", "udp -h " + (intf.empty() ? "*" : "\"" + intf + "\""));
    }

    if (properties->getIceProperty("IceDiscovery.Locator.Endpoints").empty())
    {
        properties->setProperty("IceDiscovery.Locator.AdapterId", Ice::generateUUID());
    }

    _multicastAdapter = _communicator->createObjectAdapter("IceDiscovery.Multicast");
    _replyAdapter = _communicator->createObjectAdapter("IceDiscovery.Reply");
    _locatorAdapter = _communicator->createObjectAdapter("IceDiscovery.Locator");

    //
    // Setup locator registry.
    //
    auto locatorRegistry = make_shared<LocatorRegistryI>(_communicator);
    auto locatorRegistryPrx = _locatorAdapter->addWithUUID<Ice::LocatorRegistryPrx>(locatorRegistry);

    LookupPrx lookupPrx(_communicator, "IceDiscovery/Lookup -d:" + lookupEndpoints);
    // No collocation optimization for the multicast proxy!
    lookupPrx = lookupPrx->ice_collocationOptimized(false)->ice_router(nullopt);

    //
    // Add lookup and lookup reply Ice objects
    //
    _lookup = make_shared<LookupI>(locatorRegistry, lookupPrx, properties);
    _multicastAdapter->add(_lookup, Ice::stringToIdentity("IceDiscovery/Lookup"));

    _replyAdapter->addDefaultServant(make_shared<LookupReplyI>(_lookup), "");

    _lookup->setLookupReply(_replyAdapter->createProxy<LookupReplyPrx>(Ice::Identity{"dummy", ""})->ice_datagram());

    //
    // Setup locator on the communicator.
    //
    _locator = _locatorAdapter->addWithUUID<Ice::LocatorPrx>(make_shared<LocatorI>(_lookup, locatorRegistryPrx));
    _defaultLocator = _communicator->getDefaultLocator();
    _communicator->setDefaultLocator(_locator);

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
    // Restore original default locator proxy, if the user didn't change it in the meantime.
    if (_communicator->getDefaultLocator() == _locator)
    {
        _communicator->setDefaultLocator(_defaultLocator);
    }
}
