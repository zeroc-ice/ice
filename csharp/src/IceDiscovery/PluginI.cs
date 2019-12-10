//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceDiscovery
{
    using Ice;
    using System;
    using System.Text;

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
            if (ipv4 && !preferIPv6)
            {
                address = properties.getPropertyWithDefault("IceDiscovery.Address", "239.255.0.1");
            }
            else
            {
                address = properties.getPropertyWithDefault("IceDiscovery.Address", "ff15::1");
            }
            int port = properties.getPropertyAsIntWithDefault("IceDiscovery.Port", 4061);
            string intf = properties.getProperty("IceDiscovery.Interface");

            if (properties.getProperty("IceDiscovery.Multicast.Endpoints").Length == 0)
            {
                StringBuilder s = new StringBuilder();
                s.Append("udp -h \"").Append(address).Append("\" -p ").Append(port);
                if (intf.Length != 0)
                {
                    s.Append(" --interface \"").Append(intf).Append("\"");
                }
                properties.setProperty("IceDiscovery.Multicast.Endpoints", s.ToString());
            }

            string lookupEndpoints = properties.getProperty("IceDiscovery.Lookup");
            if (lookupEndpoints.Length == 0)
            {
                int protocol = ipv4 && !preferIPv6 ? IceInternal.Network.EnableIPv4 : IceInternal.Network.EnableIPv6;
                var interfaces = IceInternal.Network.getInterfacesForMulticast(intf, protocol);
                foreach (string p in interfaces)
                {
                    if (p != interfaces[0])
                    {
                        lookupEndpoints += ":";
                    }
                    lookupEndpoints += "udp -h \"" + address + "\" -p " + port + " --interface \"" + p + "\"";
                }
            }

            if (properties.getProperty("IceDiscovery.Reply.Endpoints").Length == 0)
            {
                properties.setProperty("IceDiscovery.Reply.Endpoints",
                                       "udp -h " + (intf.Length == 0 ? "*" : "\"" + intf + "\""));
            }

            if (properties.getProperty("IceDiscovery.Locator.Endpoints").Length == 0)
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
            LocatorRegistryPrx locatorRegistryPrx = _locatorAdapter.Add(locatorRegistry);

            LookupPrx lookupPrx = LookupPrx.Parse("IceDiscovery/Lookup -d:" + lookupEndpoints, _communicator).Clone(
                clearRouter: true, collocationOptimized: false); // No colloc optimization or router for the multicast proxy!

            //
            // Add lookup and lookup reply Ice objects
            //
            LookupI lookup = new LookupI(locatorRegistry, lookupPrx, properties);
            _multicastAdapter.Add(lookup, "IceDiscovery/Lookup");

            LookupReplyTraits lookupT = default;
            LookupReplyI lookupReply = new LookupReplyI(lookup);
            _replyAdapter.AddDefaultServant(
                (current, incoming) => lookupT.Dispatch(lookupReply, current, incoming), "");
            lookup.setLookupReply(LookupReplyPrx.UncheckedCast(_replyAdapter.CreateProxy("dummy")).Clone(invocationMode: InvocationMode.Datagram));

            //
            // Setup locator on the communicator.
            //
            _locator = _locatorAdapter.Add(new LocatorI(lookup, locatorRegistryPrx));
            _defaultLocator = _communicator.getDefaultLocator();
            _communicator.setDefaultLocator(_locator);

            _multicastAdapter.Activate();
            _replyAdapter.Activate();
            _locatorAdapter.Activate();
        }

        public void destroy()
        {
            if (_multicastAdapter != null)
            {
                _multicastAdapter.Destroy();
            }
            if (_replyAdapter != null)
            {
                _replyAdapter.Destroy();
            }
            if (_locatorAdapter != null)
            {
                _locatorAdapter.Destroy();
            }
            if (_communicator.getDefaultLocator().Equals(_locator))
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

    public class Util
    {
        public static void
        registerIceDiscovery(bool loadOnInitialize)
        {
            Ice.Util.registerPluginFactory("IceDiscovery", new PluginFactory(), loadOnInitialize);
        }
    }
}
