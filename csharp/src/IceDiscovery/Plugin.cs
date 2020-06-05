//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

using ZeroC.Ice;

namespace ZeroC.IceDiscovery
{
    internal sealed class Plugin : IPlugin
    {
        private readonly Communicator _communicator;
        private ILocatorPrx? _defaultLocator;
        private ILocatorPrx? _locator;
        private ObjectAdapter? _locatorAdapter;
        private ObjectAdapter? _multicastAdapter;
        private ObjectAdapter? _replyAdapter;

        public void Destroy()
        {
            _multicastAdapter?.Destroy();
            _replyAdapter?.Destroy();
            _locatorAdapter?.Destroy();

            if (IObjectPrx.Equals(_communicator.DefaultLocator, _locator))
            {
                // Restore original default locator proxy, if the user didn't change it in the meantime
                _communicator.DefaultLocator = _defaultLocator;
            }
        }

        public void Initialize()
        {
            bool ipv4 = _communicator.GetPropertyAsBool("Ice.IPv4") ?? true;
            bool preferIPv6 = _communicator.GetPropertyAsBool("Ice.PreferIPv6Address") ?? false;
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
                if (intf.Length > 0)
                {
                    _communicator.SetProperty("IceDiscovery.Multicast.Endpoints",
                                              $"udp -h \"{address}\" -p {port} --interface \"{intf}\"");
                }
                else
                {
                    _communicator.SetProperty("IceDiscovery.Multicast.Endpoints", $"udp -h \"{address}\" -p {port}");
                }
            }

            string lookupEndpoints = _communicator.GetProperty("IceDiscovery.Lookup") ?? "";
            if (lookupEndpoints.Length == 0)
            {
                int ipVersion = ipv4 && !preferIPv6 ? Network.EnableIPv4 : Network.EnableIPv6;
                List<string> interfaces = Network.GetInterfacesForMulticast(intf, ipVersion);
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

            // Setup locatory registry.
            var locatorRegistry = new LocatorRegistry(_communicator);
            ILocatorRegistryPrx locatorRegistryPrx =
                _locatorAdapter.AddWithUUID(locatorRegistry, ILocatorRegistryPrx.Factory);

            ILookupPrx lookupPrx = ILookupPrx.Parse("IceDiscovery/Lookup -d:" + lookupEndpoints, _communicator).Clone(
                clearRouter: true,
                collocationOptimized: false); // No collocated optimization or router for the multicast proxy!

            // Add lookup Ice object
            var lookup = new Lookup(locatorRegistry, lookupPrx, _communicator, _replyAdapter);
            _multicastAdapter.Add("IceDiscovery/Lookup", lookup);

            // Setup locator on the communicator.
            _locator = _locatorAdapter.AddWithUUID(new Locator(lookup, locatorRegistryPrx), ILocatorPrx.Factory);
            _defaultLocator = _communicator.DefaultLocator;
            _communicator.DefaultLocator = _locator;

            _multicastAdapter.Activate();
            _replyAdapter.Activate();
            _locatorAdapter.Activate();
        }

        internal Plugin(Communicator communicator) => _communicator = communicator;
    }

    public sealed class PluginFactory : IPluginFactory
    {
        public static void Register(bool loadOnInitialize) =>
            Communicator.RegisterPluginFactory("IceDiscovery", new PluginFactory(), loadOnInitialize);

        public IPlugin Create(Communicator communicator, string name, string[] args) => new Plugin(communicator);
    }
}
