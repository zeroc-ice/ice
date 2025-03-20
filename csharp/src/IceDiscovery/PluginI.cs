// Copyright (c) ZeroC, Inc.

using System.Text;

namespace IceDiscovery;

public static class Util
{
    public static void registerIceDiscovery(bool loadOnInitialize) => Ice.Util.registerPluginFactory("IceDiscovery", new PluginFactory(), loadOnInitialize);
}

public sealed class PluginFactory : Ice.PluginFactory
{
    public Ice.Plugin
    create(Ice.Communicator communicator, string name, string[] args) => new PluginI(communicator);
}

public sealed class PluginI : Ice.Plugin
{
    public
    PluginI(Ice.Communicator communicator) => _communicator = communicator;

    public void initialize()
    {
        Ice.Properties properties = _communicator.getProperties();

        bool ipv4 = properties.getIcePropertyAsInt("Ice.IPv4") > 0;
        bool preferIPv6 = properties.getIcePropertyAsInt("Ice.PreferIPv6Address") > 0;
        string address = properties.getIceProperty("IceDiscovery.Address");
        if (address.Length == 0)
        {
            address = ipv4 && !preferIPv6 ? "239.255.0.1" : "ff15::1";
        }
        int port = properties.getIcePropertyAsInt("IceDiscovery.Port");
        string intf = properties.getIceProperty("IceDiscovery.Interface");

        if (properties.getIceProperty("IceDiscovery.Multicast.Endpoints").Length == 0)
        {
            var s = new StringBuilder();
            s.Append("udp -h \"").Append(address).Append("\" -p ").Append(port);
            if (intf.Length != 0)
            {
                s.Append(" --interface \"").Append(intf).Append('"');
            }
            properties.setProperty("IceDiscovery.Multicast.Endpoints", s.ToString());
        }

        string lookupEndpoints = properties.getIceProperty("IceDiscovery.Lookup");
        if (lookupEndpoints.Length == 0)
        {
            int protocol = ipv4 && !preferIPv6 ? Ice.Internal.Network.EnableIPv4 : Ice.Internal.Network.EnableIPv6;
            List<string> interfaces = Ice.Internal.Network.getInterfacesForMulticast(intf, protocol);
            foreach (string p in interfaces)
            {
                if (p != interfaces[0])
                {
                    lookupEndpoints += ":";
                }
                lookupEndpoints += "udp -h \"" + address + "\" -p " + port + " --interface \"" + p + "\"";
            }
        }

        if (properties.getIceProperty("IceDiscovery.Reply.Endpoints").Length == 0)
        {
            properties.setProperty(
                "IceDiscovery.Reply.Endpoints",
                "udp -h " + (intf.Length == 0 ? "*" : "\"" + intf + "\""));
        }

        if (properties.getIceProperty("IceDiscovery.Locator.Endpoints").Length == 0)
        {
            properties.setProperty("IceDiscovery.Locator.AdapterId", Guid.NewGuid().ToString());
        }

        _multicastAdapter = _communicator.createObjectAdapter("IceDiscovery.Multicast");
        _replyAdapter = _communicator.createObjectAdapter("IceDiscovery.Reply");
        _locatorAdapter = _communicator.createObjectAdapter("IceDiscovery.Locator");

        //
        // Setup locator registry.
        //
        var locatorRegistry = new LocatorRegistryI(_communicator);
        Ice.LocatorRegistryPrx locatorRegistryPrx = Ice.LocatorRegistryPrxHelper.uncheckedCast(
            _locatorAdapter.addWithUUID(locatorRegistry));

        Ice.ObjectPrx lookupPrx = _communicator.stringToProxy("IceDiscovery/Lookup -d:" + lookupEndpoints);
        // No colloc optimization or router for the multicast proxy!
        lookupPrx = lookupPrx.ice_collocationOptimized(false).ice_router(null);

        //
        // Add lookup and lookup reply Ice objects
        //
        var lookup = new LookupI(locatorRegistry, LookupPrxHelper.uncheckedCast(lookupPrx), properties);
        _multicastAdapter.add(lookup, Ice.Util.stringToIdentity("IceDiscovery/Lookup"));

        _replyAdapter.addDefaultServant(new LookupReplyI(lookup), "");
        var id = new Ice.Identity("dummy", "");
        lookup.setLookupReply(LookupReplyPrxHelper.uncheckedCast(_replyAdapter.createProxy(id).ice_datagram()));

        //
        // Setup locator on the communicator.
        //
        Ice.ObjectPrx loc;
        loc = _locatorAdapter.addWithUUID(
            new LocatorI(lookup, Ice.LocatorRegistryPrxHelper.uncheckedCast(locatorRegistryPrx)));
        _defaultLocator = _communicator.getDefaultLocator();
        _locator = Ice.LocatorPrxHelper.uncheckedCast(loc);
        _communicator.setDefaultLocator(_locator);

        _multicastAdapter.activate();
        _replyAdapter.activate();
        _locatorAdapter.activate();
    }

    public void destroy()
    {
        _multicastAdapter?.destroy();
        _replyAdapter?.destroy();
        _locatorAdapter?.destroy();
        if (_communicator.getDefaultLocator().Equals(_locator))
        {
            // Restore original default locator proxy, if the user didn't change it in the meantime
            _communicator.setDefaultLocator(_defaultLocator);
        }
    }

    private readonly Ice.Communicator _communicator;
    private Ice.ObjectAdapter _multicastAdapter;
    private Ice.ObjectAdapter _replyAdapter;
    private Ice.ObjectAdapter _locatorAdapter;
    private Ice.LocatorPrx _locator;
    private Ice.LocatorPrx _defaultLocator;
}
