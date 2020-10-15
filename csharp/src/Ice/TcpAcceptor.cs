// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    internal class TcpAcceptor : IAcceptor
    {
        public Endpoint Endpoint { get; }

        private readonly ObjectAdapter _adapter;
        private readonly IConnectionManager _manager;
        private readonly Socket _socket;
        private readonly IPEndPoint _addr;

        public async ValueTask<Connection> AcceptAsync()
        {
            Socket fd = await _socket.AcceptAsync().ConfigureAwait(false);

            ITransceiver transceiver = ((TcpEndpoint)Endpoint).CreateTransceiver(fd, _adapter.Name);

            MultiStreamTransceiverWithUnderlyingTransceiver multiStreamTranceiver = Endpoint.Protocol switch
            {
                Protocol.Ice1 => new LegacyTransceiver(transceiver, Endpoint, _adapter),
                _ => new SlicTransceiver(transceiver, Endpoint, _adapter)
            };

            return ((TcpEndpoint)Endpoint).CreateConnection(_manager, multiStreamTranceiver, null, "", _adapter);
        }

        public void Dispose() => _socket.CloseNoThrow();

        public string ToDetailedString()
        {
            var s = new StringBuilder("local address = ");
            s.Append(ToString());

            List<string> interfaces =
                Network.GetHostsForEndpointExpand(_addr.Address.ToString(), Network.EnableBoth, true);
            if (interfaces.Count != 0)
            {
                s.Append("\nlocal interfaces = ");
                s.Append(string.Join(", ", interfaces));
            }
            return s.ToString();
        }

        public override string ToString() => _addr.ToString();

        internal TcpAcceptor(TcpEndpoint endpoint, IConnectionManager manager, ObjectAdapter adapter)
        {
            _manager = manager;
            _adapter = adapter;

            _addr = Network.GetAddressForServerEndpoint(endpoint.Host,
                                                        endpoint.Port,
                                                        Network.EnableBoth);

            _socket = Network.CreateServerSocket(endpoint, _addr.AddressFamily);

            try
            {
                _socket.Bind(_addr);
                _addr = (IPEndPoint)_socket.LocalEndPoint!;
                _socket.Listen(endpoint.Communicator.GetPropertyAsInt("Ice.TCP.Backlog") ?? 511);
            }
            catch (SocketException ex)
            {
                _socket.CloseNoThrow();
                throw new TransportException(ex);
            }

            Endpoint = endpoint.Clone((ushort)_addr.Port);
        }
    }
}
