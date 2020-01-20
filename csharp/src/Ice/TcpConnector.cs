//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Net;

namespace IceInternal
{
    internal sealed class TcpConnector : IConnector
    {
        public ITransceiver connect()
        {
            return new TcpTransceiver(_instance, new StreamSocket(_instance, _proxy, _addr, _sourceAddr));
        }

        public short type() => _instance.type();

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
            HashUtil.hashAdd(ref _hashCode, _addr);
            if (_sourceAddr != null)
            {
                HashUtil.hashAdd(ref _hashCode, _sourceAddr);
            }
            HashUtil.hashAdd(ref _hashCode, _timeout);
            HashUtil.hashAdd(ref _hashCode, _connectionId);
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

            TcpConnector p = (TcpConnector)obj;
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

        public override string ToString() => Network.addrToString(_proxy == null ? _addr : _proxy.getAddress());

        public override int GetHashCode() => _hashCode;

        private ProtocolInstance _instance;
        private EndPoint _addr;
        private INetworkProxy? _proxy;
        private EndPoint? _sourceAddr;
        private int _timeout;
        private string _connectionId;
        private int _hashCode;
    }
}
