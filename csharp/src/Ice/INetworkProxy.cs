//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
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
        void BeginWrite(EndPoint endpoint, Buffer buf);
        int EndWrite(Buffer buf);

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
        void Finish(Buffer readBuffer, Buffer writeBuffer);

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

        public void BeginWrite(EndPoint endpoint, Buffer buf)
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
            buf.Resize(9, false);
            ByteBuffer.ByteOrder order = buf.B.Order();
            buf.B.Order(ByteBuffer.ByteOrder.BIG_ENDIAN); // Network byte order.
            buf.B.Position(0);
            buf.B.Put(0x04); // SOCKS version 4.
            buf.B.Put(0x01); // Command, establish a TCP/IP stream connection
            buf.B.PutShort((short)addr.Port); // Port
            buf.B.Put(addr.Address.GetAddressBytes()); // IPv4 address
            buf.B.Put(0x00); // User ID.
            buf.B.Position(0);
            buf.B.Limit(buf.Size());
            buf.B.Order(order);
        }

        public int EndWrite(Buffer buf)
        {
            // Once the request is sent, read the response
            return buf.B.HasRemaining() ? SocketOperation.Write : SocketOperation.Read;
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

        public void Finish(Buffer readBuffer, Buffer writeBuffer)
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

        public void BeginWrite(EndPoint endpoint, Buffer buf)
        {
            string addr = Network.AddrToString(endpoint);
            var str = new StringBuilder();
            str.Append("CONNECT ");
            str.Append(addr);
            str.Append(" HTTP/1.1\r\nHost: ");
            str.Append(addr);
            str.Append("\r\n\r\n");
            byte[] b = System.Text.Encoding.ASCII.GetBytes(str.ToString());

            //
            // HTTP connect request
            //
            buf.Resize(b.Length, false);
            buf.B.Position(0);
            buf.B.Put(b);
            buf.B.Position(0);
            buf.B.Limit(buf.Size());
        }

        public int EndWrite(Buffer buf)
        {
            // Once the request is sent, read the response
            return buf.B.HasRemaining() ? SocketOperation.Write : SocketOperation.Read;
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

        public void Finish(Buffer readBuffer, Buffer writeBuffer)
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
