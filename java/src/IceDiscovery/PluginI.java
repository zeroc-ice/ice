// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = communicator.getProperties()._clone();
        initData.properties.setProperty("Ice.Default.CollocationOptimized", "0");
        java.util.Map<String, String> props = initData.properties.getPropertiesForPrefix("Ice.Plugin.");
        for(String key : props.keySet())
        {
            initData.properties.setProperty(key, "");
        }
        _pluginCommunicator = Ice.Util.initialize(initData);
    }

    public void
    initialize()
    {
        Ice.Properties properties = _pluginCommunicator.getProperties();

        boolean ipv4 = properties.getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
        String address;
        if(ipv4)
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
        if(properties.getProperty("IceDiscovery.Reply.Endpoints").isEmpty())
        {
            StringBuilder s = new StringBuilder();
            s.append("udp");
            if(!intf.isEmpty())
            {
                s.append(" -h \"").append(intf).append("\"");
            }
            properties.setProperty("IceDiscovery.Reply.Endpoints", s.toString());
        }
        if(properties.getProperty("IceDiscovery.Locator.Endpoints").isEmpty())
        {
            properties.setProperty("IceDiscovery.Locator.Endpoints", "tcp -h 127.0.0.1");
        }

        Ice.ObjectAdapter multicastAdapter = _pluginCommunicator.createObjectAdapter("IceDiscovery.Multicast");
        Ice.ObjectAdapter replyAdapter = _pluginCommunicator.createObjectAdapter("IceDiscovery.Reply");
        Ice.ObjectAdapter locatorAdapter = _pluginCommunicator.createObjectAdapter("IceDiscovery.Locator");

        //
        // Setup locatory registry.
        //
        LocatorRegistryI locatorRegistry = new LocatorRegistryI(_communicator);
        Ice.LocatorRegistryPrx locatorRegistryPrx = Ice.LocatorRegistryPrxHelper.uncheckedCast(
            locatorAdapter.addWithUUID(locatorRegistry));

        String lookupEndpoints = properties.getProperty("IceDiscovery.Lookup");
        if(lookupEndpoints.isEmpty())
        {
            StringBuilder s = new StringBuilder();
            s.append("udp -h \"").append(address).append("\" -p ").append(port);
            if(!intf.isEmpty())
            {
                s.append(" --interface \"").append(intf).append("\"");
            }
            lookupEndpoints = s.toString();
        }

        Ice.ObjectPrx lookupPrx = _pluginCommunicator.stringToProxy("IceDiscovery/Lookup -d:" + lookupEndpoints);
        lookupPrx = lookupPrx.ice_collocationOptimized(false);

        //
        // Add lookup and lookup reply Ice objects
        //
        LookupI lookup = new LookupI(locatorRegistry, LookupPrxHelper.uncheckedCast(lookupPrx), properties);
        multicastAdapter.add(lookup, _pluginCommunicator.stringToIdentity("IceDiscovery/Lookup"));

        Ice.ObjectPrx lookupReply = replyAdapter.addWithUUID(new LookupReplyI(lookup)).ice_datagram();
        lookup.setLookupReply(LookupReplyPrxHelper.uncheckedCast(lookupReply));

        //
        // Setup locator on the communicator.
        //
        Ice.ObjectPrx locator = locatorAdapter.addWithUUID(new LocatorI(lookup, locatorRegistryPrx));
        _communicator.setDefaultLocator(
            Ice.LocatorPrxHelper.uncheckedCast(_communicator.stringToProxy(locator.toString())));
    
        multicastAdapter.activate();
        replyAdapter.activate();
        locatorAdapter.activate();
    }

    public void
    destroy()
    {
        _pluginCommunicator.destroy();
    }

    private Ice.Communicator _communicator;
    private Ice.Communicator _pluginCommunicator;
}
