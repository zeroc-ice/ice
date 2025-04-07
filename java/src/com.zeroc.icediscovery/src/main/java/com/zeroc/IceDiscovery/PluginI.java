// Copyright (c) ZeroC, Inc.

package com.zeroc.IceDiscovery;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.LocatorPrx;
import com.zeroc.Ice.LocatorRegistryPrx;
import com.zeroc.Ice.Network;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import java.util.List;
import java.util.UUID;

class PluginI implements Plugin {
    public PluginI(Communicator communicator) {
        _communicator = communicator;
    }

    @Override
    public void initialize() {
        Properties properties = _communicator.getProperties();

        boolean ipv4 = properties.getIcePropertyAsInt("Ice.IPv4") > 0;
        boolean preferIPv6 = properties.getIcePropertyAsInt("Ice.PreferIPv6Address") > 0;
        String address = properties.getIceProperty("IceDiscovery.Address");
        if (address.isEmpty()) {
            address = ipv4 && !preferIPv6 ? "239.255.0.1" : "ff15::1";
        }
        int port = properties.getIcePropertyAsInt("IceDiscovery.Port");
        String intf = properties.getIceProperty("IceDiscovery.Interface");

        if (properties.getIceProperty("IceDiscovery.Multicast.Endpoints").isEmpty()) {
            StringBuilder s = new StringBuilder();
            s.append("udp -h \"").append(address).append("\" -p ").append(port);
            if (!intf.isEmpty()) {
                s.append(" --interface \"").append(intf).append("\"");
            }
            properties.setProperty("IceDiscovery.Multicast.Endpoints", s.toString());
        }

        String lookupEndpoints = properties.getIceProperty("IceDiscovery.Lookup");
        if (lookupEndpoints.isEmpty()) {
            int protocol = ipv4 && !preferIPv6 ? Network.EnableIPv4 : Network.EnableIPv6;
            List<String> interfaces = Network.getInterfacesForMulticast(intf, protocol);
            for (String p : interfaces) {
                if (p != interfaces.get(0)) {
                    lookupEndpoints += ":";
                }
                lookupEndpoints +=
                        "udp -h \"" + address + "\" -p " + port + " --interface \"" + p + "\"";
            }
        }

        if (properties.getIceProperty("IceDiscovery.Reply.Endpoints").isEmpty()) {
            properties.setProperty(
                    "IceDiscovery.Reply.Endpoints",
                    "udp -h " + (intf.isEmpty() ? "*" : "\"" + intf + "\""));
        }

        if (properties.getIceProperty("IceDiscovery.Locator.Endpoints").isEmpty()) {
            properties.setProperty(
                    "IceDiscovery.Locator.AdapterId", UUID.randomUUID().toString());
        }

        _multicastAdapter = _communicator.createObjectAdapter("IceDiscovery.Multicast");
        _replyAdapter = _communicator.createObjectAdapter("IceDiscovery.Reply");
        _locatorAdapter = _communicator.createObjectAdapter("IceDiscovery.Locator");

        // Setup locator registry.
        LocatorRegistryI locatorRegistry = new LocatorRegistryI(_communicator);
        LocatorRegistryPrx locatorRegistryPrx =
                LocatorRegistryPrx.uncheckedCast(
                        _locatorAdapter.addWithUUID(locatorRegistry));

        ObjectPrx lookupPrx =
                _communicator.stringToProxy("IceDiscovery/Lookup -d:" + lookupEndpoints);
        // No collocation optimization for the multicast proxy!
        lookupPrx = lookupPrx.ice_collocationOptimized(false).ice_router(null);

        // Add lookup and lookup reply Ice objects
        LookupI lookup =
                new LookupI(locatorRegistry, LookupPrx.uncheckedCast(lookupPrx), properties);
        _multicastAdapter.add(lookup, Util.stringToIdentity("IceDiscovery/Lookup"));

        _replyAdapter.addDefaultServant(new LookupReplyI(lookup), "");
        final Identity id = new Identity("dummy", "");
        lookup.setLookupReply(
                LookupReplyPrx.uncheckedCast(_replyAdapter.createProxy(id).ice_datagram()));

        // Setup locator on the communicator.
        ObjectPrx locator =
                _locatorAdapter.addWithUUID(new LocatorI(lookup, locatorRegistryPrx));
        _defaultLocator = _communicator.getDefaultLocator();
        _locator = LocatorPrx.uncheckedCast(locator);
        _communicator.setDefaultLocator(_locator);

        _multicastAdapter.activate();
        _replyAdapter.activate();
        _locatorAdapter.activate();
    }

    @Override
    public void destroy() {
        if (_multicastAdapter != null) {
            _multicastAdapter.destroy();
        }
        if (_replyAdapter != null) {
            _replyAdapter.destroy();
        }
        if (_locatorAdapter != null) {
            _locatorAdapter.destroy();
        }
        if (_communicator.getDefaultLocator().equals(_locator)) {
            // Restore original default locator proxy, if the user didn't change it in the meantime
            _communicator.setDefaultLocator(_defaultLocator);
        }
    }

    private final Communicator _communicator;
    private ObjectAdapter _multicastAdapter;
    private ObjectAdapter _replyAdapter;
    private ObjectAdapter _locatorAdapter;
    private LocatorPrx _locator;
    private LocatorPrx _defaultLocator;
}
