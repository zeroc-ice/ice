// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Net;

namespace ZeroC.Ice
{
    internal sealed class UdpConnector : IConnector
    {
        private readonly UdpEndpoint _endpoint;
        private readonly EndPoint _addr;
        private readonly int _hashCode;

        public Connection Connect(string connectionId)
        {
            var transceiver = new UdpTransceiver(_endpoint.Communicator,
                                                 this,
                                                 _addr,
                                                 _endpoint.SourceAddress,
                                                 _endpoint.MulticastInterface,
                                                 _endpoint.MulticastTtl);

            return new UdpConnection(_endpoint.Communicator.OutgoingConnectionFactory,
                                     _endpoint,
                                     new LegacyTransceiver(transceiver, _endpoint, null),
                                     this,
                                     connectionId,
                                     null);
        }

        // Only for use by UdpEndpointI
        internal UdpConnector(UdpEndpoint endpoint, EndPoint addr)
        {
            _endpoint = endpoint;
            _addr = addr;

            var hash = new System.HashCode();
            hash.Add(_addr);
            if (_endpoint.SourceAddress != null)
            {
                hash.Add(_endpoint.SourceAddress);
            }
            hash.Add(_endpoint.MulticastInterface);
            hash.Add(_endpoint.MulticastTtl);
            _hashCode = hash.ToHashCode();
        }

        public override bool Equals(object? obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }

            if (obj is UdpConnector udpConnector)
            {
                if (_endpoint.MulticastInterface != udpConnector._endpoint.MulticastInterface)
                {
                    return false;
                }

                if (_endpoint.MulticastTtl != udpConnector._endpoint.MulticastTtl)
                {
                    return false;
                }

                if (!Equals(_endpoint.SourceAddress, udpConnector._endpoint.SourceAddress))
                {
                    return false;
                }

                return _addr.Equals(udpConnector._addr);
            }
            else
            {
                return false;
            }
        }

        public override string ToString() => _addr.ToString()!;

        public override int GetHashCode() => _hashCode;
    }
}
