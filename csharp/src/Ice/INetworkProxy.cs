//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Linq;

namespace ZeroC.Ice
{
    public interface INetworkProxy
    {
        //
        // Write the connection request on the connection established
        // with the network proxy server. This is called right after
        // the connection establishment succeeds.
        //
        void BeginWrite(EndPoint endpoint, IList<ArraySegment<byte>> buffer);
        int EndWrite(IList<ArraySegment<byte>> buffer, int bytesTransferred);

        //
        // Once the connection request has been sent, this is called
        // to prepare and read the response from the proxy server.
        //
        ArraySegment<byte> BeginRead();
        int EndRead(ref ArraySegment<byte> buffer, int offset);

        //
        // This is called when the response from the proxy has been
        // read. The proxy should copy the extra read data (if any) in the
        // given byte vector.
        //
        void Finish(ArraySegment<byte> readBuffer);

        //
        // If the proxy host needs to be resolved, this should return
        // a new NetworkProxy containing the IP address of the proxy.
        // This is called from the endpoint host resolver thread, so
        // it's safe if this method blocks.
        //
        ValueTask<INetworkProxy> ResolveHostAsync(int ipVersion);

        //
        // Returns the IP address of the network proxy. This method
        // must not block. It's only called on a network proxy object
        // returned by resolveHost().
        //
        EndPoint GetAddress();

        //
        // Returns the name of the proxy, used for tracing purposes.
        //
        string GetName();

        //
        // Returns the IP version(s) supported by the proxy.
        //
        int GetIPVersion();
    }

    internal sealed class SOCKSNetworkProxy : INetworkProxy
    {
        public SOCKSNetworkProxy(string host, int port)
        {
            _host = host;
            _port = port;
        }

        private SOCKSNetworkProxy(EndPoint address) => _address = address;

        public void BeginWrite(EndPoint endpoint, IList<ArraySegment<byte>> buffer)
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
            buffer.Add(data);
        }

        public int EndWrite(IList<ArraySegment<byte>> buffer, int bytesTransferred) =>
            // Once the request is sent, read the response
            bytesTransferred < buffer.GetByteCount() ? SocketOperation.Write : SocketOperation.Read;

        // Read the SOCKS4 response whose size is 8 bytes.
        public ArraySegment<byte> BeginRead() => new ArraySegment<byte>(new byte[8]);

        public int EndRead(ref ArraySegment<byte> buffer, int offset) =>
            // We're done once we read the response
            offset < buffer.Count ? SocketOperation.Read : SocketOperation.None;

        public void Finish(ArraySegment<byte> buffer)
        {
            if (buffer[0] != 0x00 || buffer[1] != 0x5a)
            {
                throw new ConnectFailedException();
            }
        }

        public async ValueTask<INetworkProxy> ResolveHostAsync(int ipVersion)
        {
            Debug.Assert(_host != null);

            // Get addresses in random order and use the first one
            IEnumerable<IPEndPoint> addresses = await Network.GetAddressesForClientEndpointAsync(_host, _port,
                ipVersion, EndpointSelectionType.Random, false).ConfigureAwait(false);
            return new SOCKSNetworkProxy(addresses.First());
        }

        public EndPoint GetAddress()
        {
            Debug.Assert(_address != null); // Host must be resolved.
            return _address;
        }

        public string GetName() => "SOCKS";

        public int GetIPVersion() => Network.EnableIPv4;

        private readonly string? _host;
        private readonly int _port;
        private readonly EndPoint? _address;
    }

    internal sealed class HTTPNetworkProxy : INetworkProxy
    {
        public HTTPNetworkProxy(string host, int port)
        {
            _host = host;
            _port = port;
            _ipVersion = Network.EnableBoth;
        }

        private HTTPNetworkProxy(EndPoint address, int ipVersion)
        {
            _address = address;
            _ipVersion = ipVersion;
        }

        public void BeginWrite(EndPoint endpoint, IList<ArraySegment<byte>> buffer)
        {
            string addr = Network.AddrToString(endpoint);
            var str = new System.Text.StringBuilder();
            str.Append("CONNECT ");
            str.Append(addr);
            str.Append(" HTTP/1.1\r\nHost: ");
            str.Append(addr);
            str.Append("\r\n\r\n");

            // HTTP connect request
            buffer.Add(System.Text.Encoding.ASCII.GetBytes(str.ToString()));
        }

        public int EndWrite(IList<ArraySegment<byte>> buffer, int bytesTransferred) =>
            // Once the request is sent, read the response
            bytesTransferred < buffer.GetByteCount() ? SocketOperation.Write : SocketOperation.Read;

        // Read the HTTP response, reserve enough space for reading at least HTTP1.1
        public ArraySegment<byte> BeginRead() => new ArraySegment<byte>(new byte[256], 0, 7);

        public int EndRead(ref ArraySegment<byte> buffer, int offset)
        {
            Debug.Assert(buffer.Offset == 0);
            //
            // Check if we received the full HTTP response, if not, continue
            // reading otherwise we're done.
            //
            int end = HttpParser.IsCompleteMessage(buffer.AsSpan(0, offset));
            if (end < 0 && offset == buffer.Count)
            {
                //
                // Read one more byte, we can't easily read bytes in advance
                // since the transport implementation might be able to read
                // the data from the memory instead of the socket.
                //
                if (offset == buffer.Array!.Length)
                {
                    // We need to allocate a new buffer
                    var newBuffer = new ArraySegment<byte>(new byte[buffer.Array.Length * 2], 0, offset + 1);
                    Buffer.BlockCopy(buffer.Array, 0, newBuffer.Array!, 0, offset);
                    buffer = newBuffer;
                }
                else
                {
                    buffer = new ArraySegment<byte>(buffer.Array, 0, offset + 1);
                }
                return SocketOperation.Read;
            }
            return SocketOperation.None;
        }

        public void Finish(ArraySegment<byte> readBuffer)
        {
            var parser = new HttpParser();
            parser.Parse(readBuffer);
            if (parser.Status() != 200)
            {
                throw new ConnectFailedException();
            }
        }

        public async ValueTask<INetworkProxy> ResolveHostAsync(int ipVersion)
        {
            Debug.Assert(_host != null);

            // Get addresses in random order and use the first one
            IEnumerable<IPEndPoint> addresses = await Network.GetAddressesForClientEndpointAsync(_host, _port,
                ipVersion, EndpointSelectionType.Random, false).ConfigureAwait(false);
            return new HTTPNetworkProxy(addresses.First(), ipVersion);
        }

        public EndPoint GetAddress()
        {
            Debug.Assert(_address != null); // Host must be resolved.
            return _address;
        }

        public string GetName() => "HTTP";

        public int GetIPVersion() => _ipVersion;

        private readonly string? _host;
        private readonly int _port;
        private readonly EndPoint? _address;
        private readonly int _ipVersion;
    }
}
