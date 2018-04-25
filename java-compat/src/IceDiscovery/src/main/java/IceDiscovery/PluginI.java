// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceDiscovery;

public class PluginI implements Ice.Plugin
{
    public
    PluginI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    @Override
    public void
    initialize()
    {
        Ice.Properties properties = _communicator.getProperties();

        boolean ipv4 = properties.getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
        boolean preferIPv6 = properties.getPropertyAsInt("Ice.PreferIPv6Address") > 0;
        String address;
        if(ipv4 && !preferIPv6)
        {
            address = properties.getPropertyWithDefault("IceDiscovery.Address", "239.255.0.1");
        }
        else
        {
            address = properties.getPropertyWithDefault("IceDiscovery.Address", "ff15::1");
        }
        int port = properties.getPropertyAsIntWithDefault("IceDiscovery.Port", 4061);
        String intf = properties.getProperty("IceDiscovery.Interface");

        if(properties.getProperty("IceDiscovery.Multicast.Endpoints").isEmpty())
        {
            StringBuilder s = new StringBuilder();
            s.append("udp -h \"").append(address).append("\" -p ").append(port);
            if(!intf.isEmpty())
            {
                s.append(" --interface \"").append(intf).append("\"");
            }
            properties.setProperty("IceDiscovery.Multicast.Endpoints", s.toString());
        }

        String lookupEndpoints = properties.getProperty("IceDiscovery.Lookup");
        if(lookupEndpoints.isEmpty())
        {
            int protocol = ipv4 && !preferIPv6 ? IceInternal.Network.EnableIPv4 : IceInternal.Network.EnableIPv6;
            java.util.List<String> interfaces = IceInternal.Network.getInterfacesForMulticast(intf, protocol);
            for(String p : interfaces)
            {
                if(p != interfaces.get(0))
                {
                    lookupEndpoints += ":";
                }
                lookupEndpoints += "udp -h \"" + address + "\" -p " + port + " --interface \"" + p + "\"";
            }
        }

        if(properties.getProperty("IceDiscovery.Reply.Endpoints").isEmpty())
        {
            properties.setProperty("IceDiscovery.Reply.Endpoints",
                                   "udp -h " + (intf.isEmpty() ? "*" : "\"" + intf + "\""));
        }

        if(properties.getProperty("IceDiscovery.Locator.Endpoints").isEmpty())
        {
            properties.setProperty("IceDiscovery.Locator.AdapterId", java.util.UUID.randomUUID().toString());
        }

        _multicastAdapter = _communicator.createObjectAdapter("IceDiscovery.Multicast");
        _replyAdapter = _communicator.createObjectAdapter("IceDiscovery.Reply");
        _locatorAdapter = _communicator.createObjectAdapter("IceDiscovery.Locator");

        //
        // Setup locatory registry.
        //
        LocatorRegistryI locatorRegistry = new LocatorRegistryI(_communicator);
        Ice.LocatorRegistryPrx locatorRegistryPrx = Ice.LocatorRegistryPrxHelper.uncheckedCast(
            _locatorAdapter.addWithUUID(locatorRegistry));

        Ice.ObjectPrx lookupPrx = _communicator.stringToProxy("IceDiscovery/Lookup -d:" + lookupEndpoints);
        // No collocation optimization for the multicast proxy!
        lookupPrx = lookupPrx.ice_collocationOptimized(false).ice_router(null);

        //
        // Add lookup and lookup reply Ice objects
        //
        LookupI lookup = new LookupI(locatorRegistry, LookupPrxHelper.uncheckedCast(lookupPrx), properties);
        _multicastAdapter.add(lookup, Ice.Util.stringToIdentity("IceDiscovery/Lookup"));

        _replyAdapter.addDefaultServant(new LookupReplyI(lookup), "");
        final Ice.Identity id = new Ice.Identity("dummy", "");
        lookup.setLookupReply(LookupReplyPrxHelper.uncheckedCast(_replyAdapter.createProxy(id).ice_datagram()));

        //
        // Setup locator on the communicator.
        //
        Ice.ObjectPrx locator = _locatorAdapter.addWithUUID(new LocatorI(lookup, locatorRegistryPrx));
        _defaultLocator = _communicator.getDefaultLocator();
        _locator = Ice.LocatorPrxHelper.uncheckedCast(locator);
        _communicator.setDefaultLocator(_locator);

        _multicastAdapter.activate();
        _replyAdapter.activate();
        _locatorAdapter.activate();
    }

    @Override
    public void
    destroy()
    {
        if(_multicastAdapter != null)
        {
            _multicastAdapter.destroy();
        }
        if(_replyAdapter != null)
        {
            _replyAdapter.destroy();
        }
        if(_locatorAdapter != null)
        {
            _locatorAdapter.destroy();
        }
        if(_communicator.getDefaultLocator().equals(_locator))
        {
            // Restore original default locator proxy, if the user didn't change it in the meantime
            _communicator.setDefaultLocator(_defaultLocator);
        }
    }

    private Ice.Communicator _communicator;
    private Ice.ObjectAdapter _multicastAdapter;
    private Ice.ObjectAdapter _replyAdapter;
    private Ice.ObjectAdapter _locatorAdapter;
    private Ice.LocatorPrx _locator;
    private Ice.LocatorPrx _defaultLocator;
}
