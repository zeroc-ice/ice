//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;
using System.Text;

namespace ZeroC.Ice
{
    internal class TcpAcceptor : IAcceptor
    {
        // TODO: add support for multiple endpoint listening on the same tcp port (possibly tcp/ssl/ws)
        public Endpoint Endpoint { get; }

        private readonly ObjectAdapter _adapter;
        private readonly Socket _fd;
        private readonly IPEndPoint _addr;

        public async ValueTask<ITransceiver> AcceptAsync()
        {
            Socket fd = await _fd.AcceptAsync().ConfigureAwait(false);

            // TODO: read data from the socket to figure out if were are accepting a tcp/ssl/ws connection.

            return ((TcpEndpoint)Endpoint).CreateTransceiver(new StreamSocket(Endpoint.Communicator, fd),
                                                             _adapter.Name);
        }

        public void Dispose() => Network.CloseSocketNoThrow(_fd);

        public string ToDetailedString()
        {
            var s = new StringBuilder("local address = ");
            s.Append(ToString());

            List<string> interfaces =
                Network.GetHostsForEndpointExpand(_addr.Address.ToString(), Endpoint.Communicator.IPVersion, true);
            if (interfaces.Count != 0)
            {
                s.Append("\nlocal interfaces = ");
                s.Append(string.Join(", ", interfaces));
            }
            return s.ToString();
        }

        public override string ToString() => Network.AddrToString(_addr);

        internal TcpAcceptor(TcpEndpoint endpoint, ObjectAdapter adapter)
        {
            _adapter = adapter;

            _addr = Network.GetAddressForServerEndpoint(endpoint.Host,
                                                        endpoint.Port,
                                                        endpoint.Communicator.IPVersion,
                                                        endpoint.Communicator.PreferIPv6);

            _fd = Network.CreateServerSocket(false, _addr.AddressFamily, endpoint.Communicator.IPVersion);
            Network.SetBlock(_fd, false);
            Network.SetTcpBufSize(_fd, endpoint.Communicator);

            _addr = Network.DoBind(_fd, _addr);
            Network.DoListen(_fd, endpoint.Communicator.GetPropertyAsInt("Ice.TCP.Backlog") ?? 511);

            Endpoint = endpoint.NewPort((ushort)_addr.Port);
        }
    }
}
