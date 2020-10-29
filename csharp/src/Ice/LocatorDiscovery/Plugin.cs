// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;

namespace ZeroC.Ice.LocatorDiscovery
{
    /// <summary>The LocatorDiscovery plug-in's factory.</summary>
    public sealed class PluginFactory : IPluginFactory
    {
        /// <inheritdoc/>
        public IPlugin Create(Communicator communicator, string name, string[] args) => new Plugin(name, communicator);
    }

    internal class Plugin : IPlugin
    {
        private readonly Communicator _communicator;
        private ILocatorPrx? _defaultLocator;
        private Locator? _locatorServant;
        private ObjectAdapter? _locatorAdapter;
        private ILocatorPrx? _locator;
        private readonly string _name;
        private ObjectAdapter? _replyAdapter;

        public async ValueTask DisposeAsync()
        {
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

            string? lookupEndpoints = _communicator.GetProperty($"{_name}.Lookup");
            if (lookupEndpoints == null)
            {
                List<string> endpoints = new ();
                List<string> ipv4Interfaces = Network.GetInterfacesForMulticast("0.0.0.0", Network.EnableIPv4);
                List<string> ipv6Interfaces = Network.GetInterfacesForMulticast("::0", Network.EnableIPv6);

                endpoints.AddRange(ipv4Interfaces.Select(i => $"{defaultIPv4Endpoint} --interface \"{i}\""));
                endpoints.AddRange(ipv6Interfaces.Select(i => $"{defaultIPv6Endpoint} --interface \"{i}\""));

                lookupEndpoints = string.Join(":", endpoints);
            }

            if (_communicator.GetProperty($"{_name}.Reply.Endpoints") == null)
            {
                _communicator.SetProperty($"{_name}.Reply.Endpoints", "udp -h \"::0\" -p 0");
            }
            _communicator.SetProperty($"{_name}.Reply.ProxyOptions", "-d");

            if (_communicator.GetProperty($"{_name}.Locator.Endpoints") == null)
            {
                _communicator.SetProperty($"{_name}.Locator.AdapterId", Guid.NewGuid().ToString());
            }

            _replyAdapter = _communicator.CreateObjectAdapter(_name + ".Reply");
            _locatorAdapter = _communicator.CreateObjectAdapter(_name + ".Locator");

            // We don't want those adapters to be registered with the locator so clear their locator.
            _replyAdapter.Locator = null;
            _locatorAdapter.Locator = null;

            var lookupPrx = ILookupPrx.Parse($"IceLocatorDiscovery/Lookup -d:{lookupEndpoints}", _communicator);
            lookupPrx = lookupPrx.Clone(clearRouter: false);

            var lookupReplyId = new Identity(Guid.NewGuid().ToString(), "");
            ILookupReplyPrx locatorReplyPrx = _replyAdapter.CreateProxy(lookupReplyId, ILookupReplyPrx.Factory);
            Debug.Assert(locatorReplyPrx.InvocationMode == InvocationMode.Datagram);

            _defaultLocator = _communicator.DefaultLocator;

            string instanceName = _communicator.GetProperty($"{_name}.InstanceName") ?? "";
            var locatorId = new Identity("Locator", instanceName.Length > 0 ? instanceName : Guid.NewGuid().ToString());
            _locatorServant = new Locator(_name, lookupPrx, _communicator, instanceName, locatorReplyPrx);

            _locator = _locatorAdapter.Add(locatorId, _locatorServant, ILocatorPrx.Factory);
            _communicator.DefaultLocator = _locator;

            _replyAdapter.Add(lookupReplyId, new LookupReply(_locatorServant));

            _replyAdapter.Activate();
            _locatorAdapter.Activate();
        }

        internal Plugin(string name, Communicator communicator)
        {
            _name = name;
            _communicator = communicator;
        }
    }
}
