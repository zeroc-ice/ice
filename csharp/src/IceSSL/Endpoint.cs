//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace ZeroC.IceSSL
{
    /// <summary> Endpoint represents a TLS layer over another endpoint.</summary>
    public sealed class Endpoint : Ice.Endpoint
    {
        public override string ConnectionId => _delegate.ConnectionId;
        public override bool HasCompressionFlag => _delegate.HasCompressionFlag;
        public override bool IsDatagram => _delegate.IsDatagram;
        public override bool IsSecure => _delegate.IsSecure;

        public override int Timeout => _delegate.Timeout;
        public override Ice.EndpointType Type => _delegate.Type;
        public override Ice.Endpoint Underlying => _delegate;

        private readonly Ice.Endpoint _delegate;
        private readonly Instance _instance;

        public override bool Equals(Ice.Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            if (other is Endpoint sslEndpoint)
            {
                return _delegate.Equals(sslEndpoint._delegate);
            }
            else
            {
                return false;
            }
        }

        public override int GetHashCode() => _delegate.GetHashCode();

        public override string OptionsToString() => _delegate.OptionsToString();

        public override bool Equivalent(Ice.Endpoint endpoint)
        {
            if (endpoint is Endpoint sslEndpoint)
            {
                return _delegate.Equivalent(sslEndpoint._delegate);
            }
            else
            {
                return false;
            }
        }

        public override void IceWritePayload(Ice.OutputStream ostr) => _delegate.IceWritePayload(ostr);

        public override Ice.Endpoint NewCompressionFlag(bool compressionFlag) =>
            compressionFlag == _delegate.HasCompressionFlag ? this :
                new Endpoint(_instance, _delegate.NewCompressionFlag(compressionFlag));

        public override Ice.Endpoint NewConnectionId(string connectionId) =>
            connectionId == _delegate.ConnectionId ? this :
                new Endpoint(_instance, _delegate.NewConnectionId(connectionId));

        public override Ice.Endpoint NewTimeout(int timeout) =>
        timeout == _delegate.Timeout ? this : new Endpoint(_instance, _delegate.NewTimeout(timeout));

        public override async ValueTask<IEnumerable<IceInternal.IConnector>>
            ConnectorsAsync(Ice.EndpointSelectionType endptSelection)
        {
            string host = "";
            for (Ice.Endpoint? p = _delegate; p != null; p = p.Underlying)
            {
                if (p is Ice.IPEndpoint ipEndpoint)
                {
                    host = ipEndpoint.Host;
                    break;
                }
            }
            IEnumerable<IceInternal.IConnector> connectors =
                await _delegate.ConnectorsAsync(endptSelection).ConfigureAwait(false);
            return connectors.Select(item => new ConnectorI(_instance, item, host));
        }

        public override IEnumerable<Ice.Endpoint> ExpandHost(out Ice.Endpoint? publish)
        {
            var endpoints = _delegate.ExpandHost(out publish).Select(endpoint => GetEndpoint(endpoint));
            if (publish != null)
            {
                publish = GetEndpoint(publish);
            }
            return endpoints;
        }

        public override IEnumerable<Ice.Endpoint> ExpandIfWildcard() =>
            _delegate.ExpandIfWildcard().Select(endpoint => GetEndpoint(endpoint));

        public override IceInternal.IAcceptor GetAcceptor(string adapterName) =>
            new Acceptor(this, _instance, _delegate.GetAcceptor(adapterName)!, adapterName);

        public override IceInternal.ITransceiver? GetTransceiver() => null;

        internal Endpoint GetEndpoint(Ice.Endpoint del) => del == _delegate ? this : new Endpoint(_instance, del);

        internal Endpoint(Instance instance, Ice.Endpoint del)
        {
            _instance = instance;
            _delegate = del;
        }
    }

    internal sealed class EndpointFactoryI : IceInternal.EndpointFactoryWithUnderlying
    {
        private readonly Instance _instance;

        public EndpointFactoryI(Instance instance, Ice.EndpointType type)
            : base(instance, type) =>
            _instance = instance;

        public override IceInternal.IEndpointFactory CloneWithUnderlying(IceInternal.TransportInstance inst,
            Ice.EndpointType underlying) =>
            new EndpointFactoryI(new Instance(_instance.Engine(), inst.Type, inst.Transport), underlying);

        protected override Ice.Endpoint CreateWithUnderlying(Ice.Endpoint underlying, string endpointString,
            Dictionary<string, string?> options, bool oaEndpoint) => new Endpoint(_instance, underlying);

        protected override Ice.Endpoint ReadWithUnderlying(Ice.Endpoint underlying, Ice.InputStream istr) =>
            new Endpoint(_instance, underlying);
    }
}
