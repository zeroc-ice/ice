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
        // TODO: add support for multiple endpoint listening on the same tcp port (possibly tcp/ssl/ws)
        public Endpoint Endpoint { get; }

        private readonly ObjectAdapter _adapter;
        private readonly Socket _socket;
        private readonly IPEndPoint _addr;

        public async ValueTask<ITransceiver> AcceptAsync()
        {
            Socket fd = await _socket.AcceptAsync().ConfigureAwait(false);

            // TODO: read data from the socket to figure out if were are accepting a tcp/ssl/ws connection.

            return ((TcpEndpoint)Endpoint).CreateTransceiver(fd, _adapter.Name);
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

        internal TcpAcceptor(TcpEndpoint endpoint, ObjectAdapter adapter)
        {
            _adapter = adapter;

            _addr = Network.GetAddressForServerEndpoint(endpoint.Host,
                                                        endpoint.Port,
                                                        Network.EnableBoth,
                                                        endpoint.Communicator.PreferIPv6);

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
