//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;

using System.Net;

namespace IceInternal
{
    internal sealed class TcpConnector : IConnector
    {
        public ITransceiver Connect() =>
            new TcpTransceiver(_instance, new StreamSocket(_instance, _proxy, _addr, _sourceAddr));

        public EndpointType Type() => _instance.Type;

        //
        // Only for use by TcpEndpoint
        //
        internal TcpConnector(TransportInstance instance, EndPoint addr, INetworkProxy? proxy, IPAddress? sourceAddr,
                              int timeout, string connectionId)
        {
            _instance = instance;
            _addr = addr;
            _proxy = proxy;
            _sourceAddr = sourceAddr;
            _timeout = timeout;
            _connectionId = connectionId;

            var hash = new System.HashCode();
            hash.Add(_addr);
            if (_sourceAddr != null)
            {
                hash.Add(_sourceAddr);
            }
            hash.Add(_timeout);
            hash.Add(_connectionId);
            _hashCode = hash.ToHashCode();
        }

        public override bool Equals(object? obj)
        {
            if (!(obj is TcpConnector))
            {
                return false;
            }

            if (this == obj)
            {
                return true;
            }

            var p = (TcpConnector)obj;
            if (_timeout != p._timeout)
            {
                return false;
            }

            if (!Equals(_sourceAddr, p._sourceAddr))
            {
                return false;
            }

            if (!_connectionId.Equals(p._connectionId))
            {
                return false;
            }

            return _addr.Equals(p._addr);
        }

        public override string ToString() => Network.AddrToString(_proxy == null ? _addr : _proxy.GetAddress());

        public override int GetHashCode() => _hashCode;

        private readonly TransportInstance _instance;
        private readonly EndPoint _addr;
        private readonly INetworkProxy? _proxy;
        private readonly IPAddress? _sourceAddr;
        private readonly int _timeout;
        private readonly string _connectionId;
        private readonly int _hashCode;
    }
}
