//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Net;

namespace ZeroC.Ice
{
    internal sealed class UdpConnector : IConnector
    {
        private readonly UdpEndpoint _endpoint;
        private readonly EndPoint _addr;
        private readonly int _hashCode;

        public ITransceiver Connect() => new UdpTransceiver(_endpoint.Communicator,
                                                            _addr,
                                                            _endpoint.SourceAddress,
                                                            _endpoint.McastInterface,
                                                            _endpoint.McastTtl);

        //
        // Only for use by UdpEndpointI
        //
        internal UdpConnector(UdpEndpoint endpoint,  EndPoint addr)
        {
            _endpoint = endpoint;
            _addr = addr;

            var hash = new System.HashCode();
            hash.Add(_addr);
            if (_endpoint.SourceAddress != null)
            {
                hash.Add(_endpoint.SourceAddress);
            }
            hash.Add(_endpoint.McastInterface);
            hash.Add(_endpoint.McastTtl);
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
                if (!_endpoint.McastInterface.Equals(udpConnector._endpoint.McastInterface))
                {
                    return false;
                }

                if (_endpoint.McastTtl != udpConnector._endpoint.McastTtl)
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

        public override string ToString() => Network.AddrToString(_addr);

        public override int GetHashCode() => _hashCode;
    }
}
