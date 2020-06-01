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
        public override string Transport => _delegate.Transport;
        public override Endpoint Underlying => _delegate;

        private readonly Endpoint _delegate;
        private readonly SslEngine _engine;
        private readonly Communicator _communicator;

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
                new SslEndpoint(_communicator, _engine, _delegate.NewCompressionFlag(compressionFlag));

        public override Endpoint NewConnectionId(string connectionId) =>
            connectionId == _delegate.ConnectionId ? this :
                new SslEndpoint(_communicator, _engine, _delegate.NewConnectionId(connectionId));

        public override Endpoint NewTimeout(int timeout) =>
            timeout == _delegate.Timeout ? this : new SslEndpoint(_communicator, _engine, _delegate.NewTimeout(timeout));

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
            return connectors.Select(item => new SslConnector(_communicator, _engine, item, host));
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
            new SslAcceptor(this, _communicator, _engine, _delegate.GetAcceptor(adapterName)!, adapterName);

        public override ITransceiver? GetTransceiver() => null;

        internal SslEndpoint GetEndpoint(Endpoint del) =>
            del == _delegate ? this : new SslEndpoint(_communicator, _engine, del);

        internal SslEndpoint(Communicator communicator, SslEngine engine, Endpoint del)
        {
            _communicator = communicator;
            _engine = engine;
            _delegate = del;
        }
    }

    internal sealed class SslEndpointFactory : EndpointFactoryWithUnderlying
    {
        private SslEngine SslEngine { get; }

        public SslEndpointFactory(
            Communicator communicator,
            SslEngine engine,
            string transport,
            EndpointType type,
            EndpointType underlying)
            : base(communicator, transport, type, underlying) => SslEngine = engine;

        public override IEndpointFactory CloneWithUnderlying(
            string transport,
            EndpointType type,
            EndpointType underlying) =>
            new SslEndpointFactory(Communicator, SslEngine, transport, type, underlying);

        protected override Endpoint CreateWithUnderlying(Endpoint underlying, string endpointString,
            Dictionary<string, string?> options, bool oaEndpoint) =>
            new SslEndpoint(Communicator, SslEngine, underlying);

        protected override Endpoint ReadWithUnderlying(Endpoint underlying, InputStream istr) =>
            new SslEndpoint(Communicator, SslEngine, underlying);
    }
}
