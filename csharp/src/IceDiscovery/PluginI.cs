//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Text;

namespace IceDiscovery
{
    public sealed class PluginFactory : Ice.PluginFactory
    {
        public Plugin
        create(Communicator communicator, string name, string[] args)
        {
            return new PluginI(communicator);
        }
    }

    public sealed class PluginI : Plugin
    {
        public
        PluginI(Communicator communicator)
        {
            _communicator = communicator;
        }

        public void initialize()
        {
            bool ipv4 = (_communicator.GetPropertyAsInt("Ice.IPv4") ?? 1) > 0;
            bool preferIPv6 = _communicator.GetPropertyAsInt("Ice.PreferIPv6Address") > 0;
            string address;
            if (ipv4 && !preferIPv6)
            {
                address = _communicator.GetProperty("IceDiscovery.Address") ?? "239.255.0.1";
            }
            else
            {
                address = _communicator.GetProperty("IceDiscovery.Address") ?? "ff15::1";
            }
            int port = _communicator.GetPropertyAsInt("IceDiscovery.Port") ?? 4061;
            string intf = _communicator.GetProperty("IceDiscovery.Interface") ?? "";

            if (_communicator.GetProperty("IceDiscovery.Multicast.Endpoints") == null)
            {
                _communicator.SetProperty("IceDiscovery.Multicast.Endpoints", intf.Length > 0 ?
                    $"udp -h \"{address}\" -p {port} --interface \"{intf}\"" : $"udp -h \"{address}\" -p {port}");
            }

            string lookupEndpoints = _communicator.GetProperty("IceDiscovery.Lookup") ?? "";
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
                    lookupEndpoints += $"udp -h \"{address}\" -p {port} --interface \"{p}\"";
                }
            }

            if (_communicator.GetProperty("IceDiscovery.Reply.Endpoints") == null)
            {
                _communicator.SetProperty("IceDiscovery.Reply.Endpoints",
                    intf.Length == 0 ? "udp -h *" : $"udp -h \"{intf}\"");
            }

            if (_communicator.GetProperty("IceDiscovery.Locator.Endpoints") == null)
            {
                _communicator.SetProperty("IceDiscovery.Locator.AdapterId", Guid.NewGuid().ToString());
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
            LookupI lookup = new LookupI(locatorRegistry, lookupPrx, _communicator);
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

            LocatorPrx? defaultLocator = _communicator.getDefaultLocator();
            if (defaultLocator != null && defaultLocator.Equals(_locator))
            {
                // Restore original default locator proxy, if the user didn't change it in the meantime
                _communicator.setDefaultLocator(_defaultLocator);
            }
        }

        private Communicator _communicator;
        private ObjectAdapter? _multicastAdapter;
        private ObjectAdapter? _replyAdapter;
        private ObjectAdapter? _locatorAdapter;
        private LocatorPrx? _locator;
        private LocatorPrx? _defaultLocator;
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
