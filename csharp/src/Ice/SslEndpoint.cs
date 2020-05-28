//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary> Endpoint represents a TLS layer over another endpoint.</summary>
    public sealed class SslEndpoint : Endpoint
    {
        public override string ConnectionId => _delegate.ConnectionId;
        public override bool HasCompressionFlag => _delegate.HasCompressionFlag;
        public override bool IsDatagram => _delegate.IsDatagram;
        public override bool IsSecure => _delegate.IsSecure;

        public override int Timeout => _delegate.Timeout;
        public override EndpointType Type => _delegate.Type;
        public override Endpoint Underlying => _delegate;

        private readonly Endpoint _delegate;
        private readonly SslInstance _instance;

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            if (other is SslEndpoint sslEndpoint)
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

        public override bool Equivalent(Endpoint endpoint)
        {
            if (endpoint is SslEndpoint sslEndpoint)
            {
                return _delegate.Equivalent(sslEndpoint._delegate);
            }
            else
            {
                return false;
            }
        }

        public override void IceWritePayload(OutputStream ostr) => _delegate.IceWritePayload(ostr);

        public override Endpoint NewCompressionFlag(bool compressionFlag) =>
            compressionFlag == _delegate.HasCompressionFlag ? this :
                new SslEndpoint(_instance, _delegate.NewCompressionFlag(compressionFlag));

        public override Endpoint NewConnectionId(string connectionId) =>
            connectionId == _delegate.ConnectionId ? this :
                new SslEndpoint(_instance, _delegate.NewConnectionId(connectionId));

        public override Endpoint NewTimeout(int timeout) =>
        timeout == _delegate.Timeout ? this : new SslEndpoint(_instance, _delegate.NewTimeout(timeout));

        public override async ValueTask<IEnumerable<IConnector>>
            ConnectorsAsync(EndpointSelectionType endptSelection)
        {
            string host = "";
            for (Endpoint? p = _delegate; p != null; p = p.Underlying)
            {
                if (p is IPEndpoint ipEndpoint)
                {
                    host = ipEndpoint.Host;
                    break;
                }
            }
            IEnumerable<IConnector> connectors =
                await _delegate.ConnectorsAsync(endptSelection).ConfigureAwait(false);
            return connectors.Select(item => new SslConnector(_instance, item, host));
        }

        public override IEnumerable<Endpoint> ExpandHost(out Endpoint? publish)
        {
            var endpoints = _delegate.ExpandHost(out publish).Select(endpoint => GetEndpoint(endpoint));
            if (publish != null)
            {
                publish = GetEndpoint(publish);
            }
            return endpoints;
        }

        public override IEnumerable<Endpoint> ExpandIfWildcard() =>
            _delegate.ExpandIfWildcard().Select(endpoint => GetEndpoint(endpoint));

        public override IAcceptor GetAcceptor(string adapterName) =>
            new SslAcceptor(this, _instance, _delegate.GetAcceptor(adapterName)!, adapterName);

        public override ITransceiver? GetTransceiver() => null;

        internal SslEndpoint GetEndpoint(Endpoint del) => del == _delegate ? this : new SslEndpoint(_instance, del);

        internal SslEndpoint(SslInstance instance, Endpoint del)
        {
            _instance = instance;
            _delegate = del;
        }
    }

    internal sealed class EndpointFactoryI : EndpointFactoryWithUnderlying
    {
        private readonly SslInstance _instance;

        public EndpointFactoryI(SslInstance instance, EndpointType type)
            : base(instance, type) =>
            _instance = instance;

        public override IEndpointFactory CloneWithUnderlying(TransportInstance inst,
            EndpointType underlying) =>
            new EndpointFactoryI(new SslInstance(_instance.Engine(), inst.Type, inst.Transport), underlying);

        protected override Endpoint CreateWithUnderlying(Endpoint underlying, string endpointString,
            Dictionary<string, string?> options, bool oaEndpoint) => new SslEndpoint(_instance, underlying);

        protected override Endpoint ReadWithUnderlying(Endpoint underlying, InputStream istr) =>
            new SslEndpoint(_instance, underlying);
    }
}
