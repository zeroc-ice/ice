// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Net;

namespace ZeroC.Ice
{
    internal sealed class TcpConnector : IConnector
    {
        public Connection Connect(string connectionId)
        {
            ITransceiver transceiver = _endpoint.CreateTransceiver(this, _addr, _proxy);

            MultiStreamTransceiverWithUnderlyingTransceiver multiStreamTranceiver = _endpoint.Protocol switch
            {
                Protocol.Ice1 => new LegacyTransceiver(transceiver, _endpoint, null),
                _ => new SlicTransceiver(transceiver, _endpoint, null)
            };

            return _endpoint.CreateConnection(_endpoint.Communicator.OutgoingConnectionFactory,
                                              multiStreamTranceiver,
                                              this,
                                              connectionId,
                                              null);
        }

        internal TcpConnector(TcpEndpoint endpoint, EndPoint addr, INetworkProxy? proxy)
        {
            _endpoint = endpoint;
            _addr = addr;
            _proxy = proxy;

            var hash = new System.HashCode();
            hash.Add(_endpoint.Protocol);
            hash.Add(_endpoint.Transport);
            hash.Add(_addr);
            if (_endpoint.SourceAddress != null)
            {
                hash.Add(_endpoint.SourceAddress);
            }
            _hashCode = hash.ToHashCode();
        }

        public override bool Equals(object? obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }

            if (obj is TcpConnector tcpConnector)
            {
                if (_endpoint.Protocol != tcpConnector._endpoint.Protocol)
                {
                    return false;
                }

                if (_endpoint.Transport != tcpConnector._endpoint.Transport)
                {
                    return false;
                }

                if (!Equals(_endpoint.SourceAddress, tcpConnector._endpoint.SourceAddress))
                {
                    return false;
                }

                return _addr.Equals(tcpConnector._addr);
            }
            else
            {
                return false;
            }
        }

        public override string ToString() => (_proxy?.Address ?? _addr).ToString()!;

        public override int GetHashCode() => _hashCode;

        private readonly TcpEndpoint _endpoint;
        private readonly EndPoint _addr;
        private readonly INetworkProxy? _proxy;
        private readonly int _hashCode;
    }
}
