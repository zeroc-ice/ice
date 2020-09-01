//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    public interface INetworkProxy
    {
        // Returns the IP address of the network proxy. This method should not block. It's only called on a network
        // proxy object returned by ResolveHostAsync.
        EndPoint Address { get; }

        // Returns the name of the proxy, used for tracing purposes.
        string Name { get; }

        // Returns the IP version(s) supported by the proxy.
        int IPVersion { get; }

        // Connect to the server through the network proxy.
        ValueTask ConnectAsync(Socket socket, EndPoint endpoint, CancellationToken cancel);

        // If the proxy host needs to be resolved, this should return a new NetworkProxy containing the IP address
        // of the proxy.
        ValueTask<INetworkProxy> ResolveHostAsync(int ipVersion, CancellationToken cancel);
    }

    internal sealed class SOCKSNetworkProxy : INetworkProxy
    {
        public EndPoint Address => _address!;
        public string Name => "SOCKS";
        public int IPVersion => Network.EnableIPv4;

        private readonly string? _host;
        private readonly int _port;
        private readonly EndPoint? _address;

        public async ValueTask ConnectAsync(Socket socket, EndPoint endpoint, CancellationToken cancel)
        {
            if (!(endpoint is IPEndPoint))
            {
                throw new TransportException("SOCKS4 does not support domain names");
            }
            else if (endpoint.AddressFamily != AddressFamily.InterNetwork)
            {
                throw new TransportException("SOCKS4 only supports IPv4 addresses");
            }

            //
            // SOCKS connect request
            //
            var addr = (IPEndPoint)endpoint;
            byte[] data = new byte[9];
            data[0] = 0x04; // SOCKS version 4.
            data[1] = 0x01; // Command, establish a TCP/IP stream connection

            Debug.Assert(BitConverter.IsLittleEndian);
            short port = BinaryPrimitives.ReverseEndianness((short)addr.Port); // Network byte order (BIG_ENDIAN)
            MemoryMarshal.Write(data.AsSpan(2, 2), ref port); // Port
            Buffer.BlockCopy(addr.Address.GetAddressBytes(), 0, data, 4, 4); // IPv4 address
            data[8] = 0x00; // User ID.

            // Send the request.
            await socket.SendAsync(data, SocketFlags.None, cancel);

            // Now wait for the response whose size is 8 bytes.
            await socket.ReceiveAsync(data.AsMemory(0, 8), SocketFlags.None, cancel);
            if (data[0] != 0x00 || data[1] != 0x5a)
            {
                throw new ConnectFailedException();
            }
        }

        public async ValueTask<INetworkProxy> ResolveHostAsync(int ipVersion, CancellationToken cancel)
        {
            Debug.Assert(_host != null);

            // Get addresses in random order and use the first one
            IEnumerable<IPEndPoint> addresses =
                await Network.GetAddressesForClientEndpointAsync(_host,
                                                                 _port,
                                                                 ipVersion,
                                                                 EndpointSelectionType.Random,
                                                                 false,
                                                                 cancel).ConfigureAwait(false);
            return new SOCKSNetworkProxy(addresses.First());
        }

        internal SOCKSNetworkProxy(string host, int port)
        {
            _host = host;
            _port = port;
        }

        private SOCKSNetworkProxy(EndPoint address) => _address = address;
    }

    internal sealed class HTTPNetworkProxy : INetworkProxy
    {
        public EndPoint Address => _address!;
        public string Name => "HTTP";
        public int IPVersion { get; }

        private readonly string? _host;
        private readonly int _port;
        private readonly EndPoint? _address;

        public async ValueTask ConnectAsync(Socket socket, EndPoint endpoint, CancellationToken cancel)
        {
            // HTTP connect request
            string addr = Network.AddrToString(endpoint);
            var str = new System.Text.StringBuilder();
            str.Append("CONNECT ");
            str.Append(addr);
            str.Append(" HTTP/1.1\r\nHost: ");
            str.Append(addr);
            str.Append("\r\n\r\n");

            // Send the connect request.
            await socket.SendAsync(System.Text.Encoding.ASCII.GetBytes(str.ToString()), SocketFlags.None, cancel);

            // Read the HTTP response, reserve enough space for reading at least HTTP1.1
            byte[] buffer = new byte[256];
            int received = 0;
            while (true)
            {
                received += await socket.ReceiveAsync(buffer.AsMemory(received), SocketFlags.None, cancel);

                //
                // Check if we received the full HTTP response, if not, continue reading otherwise we're done.
                //
                int end = HttpParser.IsCompleteMessage(buffer.AsSpan(0, received));
                if (end < 0 && received == buffer.Length)
                {
                    // We need to allocate a new buffer
                    byte[] newBuffer = new byte[buffer.Length * 2];
                    Buffer.BlockCopy(buffer, 0, newBuffer, 0, received);
                    buffer = newBuffer;
                }
                else
                {
                    break;
                }
            }

            var parser = new HttpParser();
            parser.Parse(buffer);
            if (parser.Status() != 200)
            {
                throw new ConnectFailedException();
            }
        }

        public async ValueTask<INetworkProxy> ResolveHostAsync(int ipVersion, CancellationToken cancel)
        {
            Debug.Assert(_host != null);

            // Get addresses in random order and use the first one
            IEnumerable<IPEndPoint> addresses =
                await Network.GetAddressesForClientEndpointAsync(_host,
                                                                 _port,
                                                                 ipVersion,
                                                                 EndpointSelectionType.Random,
                                                                 false,
                                                                 cancel).ConfigureAwait(false);
            return new HTTPNetworkProxy(addresses.First(), ipVersion);
        }

        internal HTTPNetworkProxy(string host, int port)
        {
            _host = host;
            _port = port;
            IPVersion = Network.EnableBoth;
        }

        private HTTPNetworkProxy(EndPoint address, int ipVersion)
        {
            _address = address;
            IPVersion = ipVersion;
        }
    }
}
