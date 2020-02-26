//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using Ice;
using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;

namespace IceInternal
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
        void BeginRead(Buffer buf);
        int EndRead(Buffer buf);

        //
        // This is called when the response from the proxy has been
        // read. The proxy should copy the extra read data (if any) in the
        // given byte vector.
        //
        void Finish(Buffer readBuffer);

        //
        // If the proxy host needs to be resolved, this should return
        // a new NetworkProxy containing the IP address of the proxy.
        // This is called from the endpoint host resolver thread, so
        // it's safe if this this method blocks.
        //
        INetworkProxy ResolveHost(int ipVersion);

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

    public sealed class SOCKSNetworkProxy : INetworkProxy
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
                throw new Ice.FeatureNotSupportedException("SOCKS4 does not support domain names");
            }
            else if (endpoint.AddressFamily != AddressFamily.InterNetwork)
            {
                throw new Ice.FeatureNotSupportedException("SOCKS4 only supports IPv4 addresses");
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
            System.Buffer.BlockCopy(addr.Address.GetAddressBytes(), 0, data, 4, 4); // IPv4 address
            data[8] = 0x00; // User ID.
            buffer.Add(data);
        }

        public int EndWrite(IList<ArraySegment<byte>> buffer, int bytesTransferred)
        {
            // Once the request is sent, read the response
            return bytesTransferred < buffer.GetBytesCount() ? SocketOperation.Write : SocketOperation.Read;
        }

        public void BeginRead(Buffer buf)
        {
            //
            // Read the SOCKS4 response whose size is 8 bytes.
            //
            buf.Resize(8, true);
            buf.B.Position(0);
        }

        public int EndRead(Buffer buf)
        {
            // We're done once we read the response
            return buf.B.HasRemaining() ? SocketOperation.Read : SocketOperation.None;
        }

        public void Finish(Buffer readBuffer)
        {
            readBuffer.B.Position(0);
            byte b1 = readBuffer.B.Get();
            byte b2 = readBuffer.B.Get();
            if (b1 != 0x00 || b2 != 0x5a)
            {
                throw new Ice.ConnectFailedException();
            }
        }

        public INetworkProxy ResolveHost(int ipVersion)
        {
            Debug.Assert(_host != null);
            return new SOCKSNetworkProxy(Network.GetAddresses(_host,
                                                              _port,
                                                              ipVersion,
                                                              Ice.EndpointSelectionType.Random,
                                                              false,
                                                              true)[0]);
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

    public sealed class HTTPNetworkProxy : INetworkProxy
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
            var str = new StringBuilder();
            str.Append("CONNECT ");
            str.Append(addr);
            str.Append(" HTTP/1.1\r\nHost: ");
            str.Append(addr);
            str.Append("\r\n\r\n");

            // HTTP connect request
            buffer.Add(Encoding.ASCII.GetBytes(str.ToString()));
        }

        public int EndWrite(IList<ArraySegment<byte>> buffer, int bytesTransferred)
        {
            // Once the request is sent, read the response
            return bytesTransferred < buffer.GetBytesCount() ? SocketOperation.Write : SocketOperation.Read;
        }

        public void BeginRead(Buffer buf)
        {
            //
            // Read the HTTP response
            //
            buf.Resize(7, true); // Enough space for reading at least HTTP1.1
            buf.B.Position(0);
        }

        public int EndRead(Buffer buf)
        {
            //
            // Check if we received the full HTTP response, if not, continue
            // reading otherwise we're done.
            //
            int end = new HttpParser().IsCompleteMessage(buf.B, 0, buf.B.Position());
            if (end < 0 && !buf.B.HasRemaining())
            {
                //
                // Read one more byte, we can't easily read bytes in advance
                // since the transport implenentation might be be able to read
                // the data from the memory instead of the socket.
                //
                buf.Resize(buf.Size() + 1, true);
                return SocketOperation.Read;
            }
            return SocketOperation.None;
        }

        public void Finish(Buffer readBuffer)
        {
            var parser = new HttpParser();
            parser.Parse(readBuffer.B, 0, readBuffer.B.Position());
            if (parser.Status() != 200)
            {
                throw new Ice.ConnectFailedException();
            }
        }

        public INetworkProxy ResolveHost(int ipVersion)
        {
            Debug.Assert(_host != null);
            return new HTTPNetworkProxy(Network.GetAddresses(_host,
                                                             _port,
                                                             ipVersion,
                                                             Ice.EndpointSelectionType.Random,
                                                             false,
                                                             true)[0],
                                        ipVersion);
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
