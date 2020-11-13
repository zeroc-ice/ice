// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Net;

namespace ZeroC.Ice
{
    internal sealed class UdpConnector : Connector
    {
        private readonly UdpEndpoint _endpoint;
        private readonly EndPoint _addr;
        private int _hashCode;

        public override Endpoint Endpoint => _endpoint;

        public override Connection Connect(string connectionId)
        {
            var transceiver = new UdpTransceiver(_endpoint.Communicator,
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

        public override bool Equals(Connector? obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }

            if (obj is UdpConnector udpConnector)
            {
                if (!_addr.Equals(udpConnector._addr))
                {
                    return false;
                }
                if (_endpoint.MulticastInterface != udpConnector._endpoint.MulticastInterface)
                {
                    return false;
                }

                if (_endpoint.MulticastTtl != udpConnector._endpoint.MulticastTtl)
                {
                    return false;
                }

                return Equals(_endpoint.SourceAddress, udpConnector._endpoint.SourceAddress);
            }
            else
            {
                return false;
            }
        }

        public override int GetHashCode()
        {
            // This code is thread safe because reading/writing _hashCode (an int) is atomic.
            if (_hashCode != 0)
            {
                // Return cached value
                return _hashCode;
            }
            else
            {
                var hash = new HashCode();
                hash.Add(_addr);
                hash.Add(_endpoint.SourceAddress);
                hash.Add(_endpoint.MulticastInterface);
                hash.Add(_endpoint.MulticastTtl);
                int hashCode = hash.ToHashCode();
                // 0 is not a valid value as it means "not initialized".
                _hashCode = hashCode == 0 ? 1 : hashCode;
                return _hashCode;
            }
        }

        public override string ToString() => _addr.ToString()!;

        // Only for use by UdpEndpoint
        internal UdpConnector(UdpEndpoint endpoint, EndPoint addr)
        {
            _endpoint = endpoint;
            _addr = addr;
        }
    }
}
