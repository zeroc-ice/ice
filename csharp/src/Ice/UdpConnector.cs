//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Net;

namespace ZeroC.Ice
{
    internal sealed class UdpConnector : IConnector
    {
        public ITransceiver Connect() =>
            new UdpTransceiver(_communicator, _addr, _sourceAddr, _mcastInterface, _mcastTtl);

        //
        // Only for use by UdpEndpointI
        //
        internal UdpConnector(
            Communicator communicator,
            EndPoint addr,
            IPAddress? sourceAddr,
            string mcastInterface,
            int mcastTtl)
        {
            _communicator = communicator;
            _addr = addr;
            _sourceAddr = sourceAddr;
            _mcastInterface = mcastInterface;
            _mcastTtl = mcastTtl;

            var hash = new System.HashCode();
            hash.Add(_addr);
            if (sourceAddr != null)
            {
                hash.Add(_sourceAddr);
            }
            hash.Add(_mcastInterface);
            hash.Add(_mcastTtl);
            _hashCode = hash.ToHashCode();
        }

        public override bool Equals(object? obj)
        {
            if (!(obj is UdpConnector))
            {
                return false;
            }

            if (this == obj)
            {
                return true;
            }

            var p = (UdpConnector)obj;
            if (!_mcastInterface.Equals(p._mcastInterface))
            {
                return false;
            }

            if (_mcastTtl != p._mcastTtl)
            {
                return false;
            }

            if (!Equals(_sourceAddr, p._sourceAddr))
            {
                return false;
            }

            return _addr.Equals(p._addr);
        }

        public override string ToString() => Network.AddrToString(_addr);

        public override int GetHashCode() => _hashCode;

        private readonly Communicator _communicator;
        private readonly EndPoint _addr;
        private readonly IPAddress? _sourceAddr;
        private readonly string _mcastInterface;
        private readonly int _mcastTtl;
        private readonly int _hashCode;
    }
}
