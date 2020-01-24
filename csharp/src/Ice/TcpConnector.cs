//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Net;

namespace IceInternal
{
    internal sealed class TcpConnector : IConnector
    {
        public ITransceiver Connect() =>
            new TcpTransceiver(_instance, new StreamSocket(_instance, _proxy, _addr, _sourceAddr));

        public short Type() => _instance.Type;

        //
        // Only for use by TcpEndpoint
        //
        internal TcpConnector(ProtocolInstance instance, EndPoint addr, INetworkProxy? proxy, EndPoint? sourceAddr,
                              int timeout, string connectionId)
        {
            _instance = instance;
            _addr = addr;
            _proxy = proxy;
            _sourceAddr = sourceAddr;
            _timeout = timeout;
            _connectionId = connectionId;

            _hashCode = 5381;
            HashUtil.HashAdd(ref _hashCode, _addr);
            if (_sourceAddr != null)
            {
                HashUtil.HashAdd(ref _hashCode, _sourceAddr);
            }
            HashUtil.HashAdd(ref _hashCode, _timeout);
            HashUtil.HashAdd(ref _hashCode, _connectionId);
        }

        public override bool Equals(object obj)
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

        private readonly ProtocolInstance _instance;
        private readonly EndPoint _addr;
        private readonly INetworkProxy? _proxy;
        private readonly EndPoint? _sourceAddr;
        private readonly int _timeout;
        private readonly string _connectionId;
        private readonly int _hashCode;
    }
}
