// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceDiscovery
{
    using System;
    using System.Text;
    using System.Collections.Generic;

    public sealed class PluginFactory : Ice.PluginFactory
    {
        public Ice.Plugin
        create(Ice.Communicator communicator, string name, string[] args)
        {
            return new PluginI(communicator);
        }
    }

    public sealed class PluginI : Ice.Plugin
    {
        public
        PluginI(Ice.Communicator communicator)
        {
            _communicator = communicator;

            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties().ice_clone_();
            initData.properties.setProperty("Ice.Default.CollocationOptimized", "0");
            Dictionary<string, string> props = initData.properties.getPropertiesForPrefix("Ice.Plugin.");
            foreach(string key in props.Keys)
            {
                initData.properties.setProperty(key, "");
            }
            _pluginCommunicator = Ice.Util.initialize(initData);
        }

        public void initialize()
        {
            Ice.Properties properties = _pluginCommunicator.getProperties();
            
            bool ipv4 = properties.getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
            string address;
            if(ipv4)
            {
                address = properties.getPropertyWithDefault("IceDiscovery.Address", "239.255.0.1");
            }
            else
            {
                address = properties.getPropertyWithDefault("IceDiscovery.Address", "ff15::1");
            }
            int port = properties.getPropertyAsIntWithDefault("IceDiscovery.Port", 4061);
            string intf = properties.getProperty("IceDiscovery.Interface");

            if(properties.getProperty("IceDiscovery.Multicast.Endpoints").Length == 0)
            {
                StringBuilder s = new StringBuilder();
                s.Append("udp -h \"").Append(address).Append("\" -p ").Append(port);
                if(intf.Length != 0)
                {
                    s.Append(" --interface \"").Append(intf).Append("\"");
                }
                properties.setProperty("IceDiscovery.Multicast.Endpoints", s.ToString());
            }
            if(properties.getProperty("IceDiscovery.Reply.Endpoints").Length == 0)
            {
                StringBuilder s = new StringBuilder();
                s.Append("udp");
                if(intf.Length != 0)
                {
                    s.Append(" -h \"").Append(intf).Append("\"");
                }
                properties.setProperty("IceDiscovery.Reply.Endpoints", s.ToString());
            }
            if(properties.getProperty("IceDiscovery.Locator.Endpoints").Length == 0)
            {
                if(ipv4)
                {
                    properties.setProperty("IceDiscovery.Locator.Endpoints", "tcp -h 127.0.0.1");
                }
                else
                {
                    properties.setProperty("IceDiscovery.Locator.Endpoints", "tcp -h \"::1\"");
                }
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

            string lookupEndpoints = properties.getProperty("IceDiscovery.Lookup");
            if(lookupEndpoints.Length == 0)
            {
                lookupEndpoints = "udp -h \"" + address + "\" -p " + port;
                if(intf.Length > 0)
                {
                    lookupEndpoints += " --interface \"" + intf + "\"";
                }
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
            Ice.ObjectPrx loc;
            loc = locatorAdapter.addWithUUID(
                new LocatorI(lookup, Ice.LocatorRegistryPrxHelper.uncheckedCast(locatorRegistryPrx)));
            _communicator.setDefaultLocator(
                Ice.LocatorPrxHelper.uncheckedCast(_communicator.stringToProxy(loc.ToString())));
    
            multicastAdapter.activate();
            replyAdapter.activate();
            locatorAdapter.activate();
        }

        public void destroy()
        {
            _pluginCommunicator.destroy();
        }

        private Ice.Communicator _communicator;
        private Ice.Communicator _pluginCommunicator;
    }

}