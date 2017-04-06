// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceDiscovery;

import com.zeroc.IceInternal.Network;

public class PluginI implements com.zeroc.Ice.Plugin
{
    public PluginI(com.zeroc.Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    @Override
    public void initialize()
    {
        com.zeroc.Ice.Properties properties = _communicator.getProperties();

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
            int protocol = ipv4 && !preferIPv6 ? Network.EnableIPv4 : Network.EnableIPv6;
            java.util.List<String> interfaces = Network.getInterfacesForMulticast(intf, protocol);
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
        com.zeroc.Ice.LocatorRegistryPrx locatorRegistryPrx = com.zeroc.Ice.LocatorRegistryPrx.uncheckedCast(
            _locatorAdapter.addWithUUID(locatorRegistry));

        com.zeroc.Ice.ObjectPrx lookupPrx = _communicator.stringToProxy("IceDiscovery/Lookup -d:" + lookupEndpoints);
        lookupPrx = lookupPrx.ice_collocationOptimized(false); // No collocation optimization for the multicast proxy!

        //
        // Add lookup and lookup reply Ice objects
        //
        LookupI lookup = new LookupI(locatorRegistry, LookupPrx.uncheckedCast(lookupPrx), properties);
        _multicastAdapter.add(lookup, com.zeroc.Ice.Util.stringToIdentity("IceDiscovery/Lookup"));

        com.zeroc.Ice.ObjectPrx lookupReply = _replyAdapter.addWithUUID(new LookupReplyI(lookup)).ice_datagram();
        lookup.setLookupReply(LookupReplyPrx.uncheckedCast(lookupReply));

        //
        // Setup locator on the communicator.
        //
        com.zeroc.Ice.ObjectPrx locator = _locatorAdapter.addWithUUID(new LocatorI(lookup, locatorRegistryPrx));
        _defaultLocator = _communicator.getDefaultLocator();
        _communicator.setDefaultLocator(com.zeroc.Ice.LocatorPrx.uncheckedCast(locator));

        _multicastAdapter.activate();
        _replyAdapter.activate();
        _locatorAdapter.activate();
    }

    @Override
    public void destroy()
    {
        _multicastAdapter.destroy();
        _replyAdapter.destroy();
        _locatorAdapter.destroy();
        _communicator.setDefaultLocator(_defaultLocator);
    }

    private com.zeroc.Ice.Communicator _communicator;
    private com.zeroc.Ice.ObjectAdapter _multicastAdapter;
    private com.zeroc.Ice.ObjectAdapter _replyAdapter;
    private com.zeroc.Ice.ObjectAdapter _locatorAdapter;
    private com.zeroc.Ice.LocatorPrx _defaultLocator;
}
