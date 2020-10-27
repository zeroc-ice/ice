// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using ZeroC.Ice;

namespace ZeroC.Ice.Discovery
{
    internal sealed class Plugin : IPlugin
    {
        private readonly Communicator _communicator;
        private ILocatorPrx? _defaultLocator;
        private ILocatorPrx? _locator;
        private ObjectAdapter? _locatorAdapter;
        private ObjectAdapter? _multicastAdapter;
        private readonly string _pluginName;
        private ObjectAdapter? _replyAdapter;

        public async ValueTask DisposeAsync()
        {
            if (_multicastAdapter != null)
            {
                await _multicastAdapter.DisposeAsync().ConfigureAwait(false);
            }

            if (_replyAdapter != null)
            {
                await _replyAdapter.DisposeAsync().ConfigureAwait(false);
            }

            if (_locatorAdapter != null)
            {
                await _locatorAdapter.DisposeAsync().ConfigureAwait(false);
            }

            if (IObjectPrx.Equals(_communicator.DefaultLocator, _locator))
            {
                // Restore original default locator proxy, if the user didn't change it in the meantime
                _communicator.DefaultLocator = _defaultLocator;
            }
        }

        public void Initialize(PluginInitializationContext context)
        {
            const string defaultIPv4Endpoint = "udp -h 239.255.0.1 -p 4061";
            const string defaultIPv6Endpoint = "udp -h \"ff15::1\" -p 4061";

            if (_communicator.GetProperty($"{_pluginName}.Multicast.Endpoints") == null)
            {
                _communicator.SetProperty($"{_pluginName}.Multicast.Endpoints",
                                          $"{defaultIPv4Endpoint}:{defaultIPv6Endpoint}");
            }

            string? lookupEndpoints = _communicator.GetProperty($"{_pluginName}.Lookup");
            if (lookupEndpoints == null)
            {
                List<string> endpoints = new ();
                List<string> ipv4Interfaces = Network.GetInterfacesForMulticast("0.0.0.0", Network.EnableIPv4);
                List<string> ipv6Interfaces = Network.GetInterfacesForMulticast("::0", Network.EnableIPv6);

                endpoints.AddRange(ipv4Interfaces.Select(i => $"{defaultIPv4Endpoint} --interface \"{i}\""));
                endpoints.AddRange(ipv6Interfaces.Select(i => $"{defaultIPv6Endpoint} --interface \"{i}\""));

                lookupEndpoints = string.Join(":", endpoints);
            }

            if (_communicator.GetProperty($"{_pluginName}.Reply.Endpoints") == null)
            {
                _communicator.SetProperty($"{_pluginName}.Reply.Endpoints", "udp -h \"::0\" -p 0");
            }
            _communicator.SetProperty($"{_pluginName}.Reply.ProxyOptions", "-d"); // create datagram proxies

            if (_communicator.GetProperty($"{_pluginName}.Locator.Endpoints") == null)
            {
                _communicator.SetProperty($"{_pluginName}.Locator.AdapterId", Guid.NewGuid().ToString());
            }

            _multicastAdapter = _communicator.CreateObjectAdapter($"{_pluginName}.Multicast");
            _replyAdapter = _communicator.CreateObjectAdapter($"{_pluginName}.Reply");
            _locatorAdapter = _communicator.CreateObjectAdapter($"{_pluginName}.Locator");

            // Setup locator registry.
            var locatorRegistryServant = new LocatorRegistry(_communicator);
            ILocatorRegistryPrx locatorRegistry =
                _locatorAdapter.AddWithUUID(locatorRegistryServant, ILocatorRegistryPrx.Factory);

            ILookupPrx lookup =
                ILookupPrx.Parse($"IceDiscovery/Lookup -d:{lookupEndpoints}", _communicator).Clone(clearRouter: true);

            // Add lookup Ice object
            var lookupServant = new Lookup(locatorRegistryServant, _pluginName, _communicator);
            _multicastAdapter.Add("IceDiscovery/Lookup", lookupServant);

            // Setup locator on the communicator.
            _locator = _locatorAdapter.AddWithUUID(new Locator(locatorRegistry, lookup, _replyAdapter, _pluginName),
                                                   ILocatorPrx.Factory);

            _defaultLocator = _communicator.DefaultLocator;
            _communicator.DefaultLocator = _locator;

            _multicastAdapter.Activate();
            _replyAdapter.Activate();
            _locatorAdapter.Activate();
        }

        internal Plugin(Communicator communicator, string name)
        {
            _communicator = communicator;
            _pluginName = name;
        }
    }

    /// <summary>The IceDiscovery plug-in's factory.</summary>
    public sealed class PluginFactory : IPluginFactory
    {
        /// <inheritdoc/>
        public IPlugin Create(Communicator communicator, string name, string[] args) =>
            new Plugin(communicator, name);
    }
}
