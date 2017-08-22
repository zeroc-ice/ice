// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
        }

        public void initialize()
        {
            Ice.Properties properties = _communicator.getProperties();

            bool ipv4 = properties.getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
            bool preferIPv6 = properties.getPropertyAsInt("Ice.PreferIPv6Address") > 0;
            string address;
            if(ipv4 && !preferIPv6)
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
                properties.setProperty("IceDiscovery.Locator.AdapterId", Guid.NewGuid().ToString());
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

            string lookupEndpoints = properties.getProperty("IceDiscovery.Lookup");
            if(lookupEndpoints.Length == 0)
            {
                lookupEndpoints = "udp -h \"" + address + "\" -p " + port;
                if(intf.Length > 0)
                {
                    lookupEndpoints += " --interface \"" + intf + "\"";
                }
            }

            Ice.ObjectPrx lookupPrx = _communicator.stringToProxy("IceDiscovery/Lookup -d:" + lookupEndpoints);
            // No colloc optimization for the multicast proxy!
            lookupPrx = lookupPrx.ice_collocationOptimized(false).ice_router(null);
            try
            {
                lookupPrx.ice_getConnection();
            }
            catch(Ice.LocalException ex)
            {
                StringBuilder b = new StringBuilder();
                b.Append("IceDiscovery is unable to establish a multicast connection:\n");
                b.Append("proxy = ");
                b.Append(lookupPrx.ToString());
                b.Append('\n');
                b.Append(ex.ToString());
                throw new Ice.PluginInitializationException(b.ToString());
            }

            //
            // Add lookup and lookup reply Ice objects
            //
            LookupI lookup = new LookupI(locatorRegistry, LookupPrxHelper.uncheckedCast(lookupPrx), properties);
            _multicastAdapter.add(lookup, _communicator.stringToIdentity("IceDiscovery/Lookup"));

            Ice.ObjectPrx lookupReply = _replyAdapter.addWithUUID(new LookupReplyI(lookup)).ice_datagram();
            lookup.setLookupReply(LookupReplyPrxHelper.uncheckedCast(lookupReply));

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
            if(_communicator.getDefaultLocator().Equals(_locator))
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

}
