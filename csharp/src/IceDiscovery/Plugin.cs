//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;

namespace IceDiscovery
{
    public sealed class PluginFactory : Ice.IPluginFactory
    {
        public IPlugin
        create(Communicator communicator, string name, string[] args) => new Plugin(communicator);
    }

    public sealed class Plugin : IPlugin
    {
        public
        Plugin(Communicator communicator) => _communicator = communicator;

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

            _multicastAdapter = _communicator.CreateObjectAdapter("IceDiscovery.Multicast");
            _replyAdapter = _communicator.CreateObjectAdapter("IceDiscovery.Reply");
            _locatorAdapter = _communicator.CreateObjectAdapter("IceDiscovery.Locator");

            //
            // Setup locatory registry.
            //
            LocatorRegistry locatorRegistry = new LocatorRegistry(_communicator);
            ILocatorRegistryPrx locatorRegistryPrx = _locatorAdapter.Add(locatorRegistry);

            ILookupPrx lookupPrx = ILookupPrx.Parse("IceDiscovery/Lookup -d:" + lookupEndpoints, _communicator).Clone(
                clearRouter: true, collocationOptimized: false); // No colloc optimization or router for the multicast proxy!

            //
            // Add lookup and lookup reply Ice objects
            //
            Lookup lookup = new Lookup(locatorRegistry, lookupPrx, _communicator);
            _multicastAdapter.Add(lookup, "IceDiscovery/Lookup");

            LookupReplyTraits lookupT = default;
            LookupReply lookupReply = new LookupReply(lookup);
            _replyAdapter.AddDefaultServant(
                (current, incoming) => lookupT.Dispatch(lookupReply, current, incoming), "");
            lookup.SetLookupReply(ILookupReplyPrx.UncheckedCast(_replyAdapter.CreateProxy("dummy")).Clone(invocationMode: InvocationMode.Datagram));

            //
            // Setup locator on the communicator.
            //
            _locator = _locatorAdapter.Add(new Locator(lookup, locatorRegistryPrx));
            _defaultLocator = _communicator.GetDefaultLocator();
            _communicator.SetDefaultLocator(_locator);

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

            ILocatorPrx? defaultLocator = _communicator.GetDefaultLocator();
            if (defaultLocator != null && defaultLocator.Equals(_locator))
            {
                // Restore original default locator proxy, if the user didn't change it in the meantime
                _communicator.SetDefaultLocator(_defaultLocator);
            }
        }

        private Communicator _communicator;
        private ObjectAdapter? _multicastAdapter;
        private ObjectAdapter? _replyAdapter;
        private ObjectAdapter? _locatorAdapter;
        private ILocatorPrx? _locator;
        private ILocatorPrx? _defaultLocator;
    }

    public class Util
    {
        public static void
        registerIceDiscovery(bool loadOnInitialize)
        {
            Communicator.RegisterPluginFactory("IceDiscovery", new PluginFactory(), loadOnInitialize);
        }
    }
}
