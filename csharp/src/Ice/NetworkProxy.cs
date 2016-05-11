// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System;
    using System.Net;
    using System.Net.Sockets;
    using System.Diagnostics;
    using System.Text;

    public interface NetworkProxy
    {
        //
        // Write the connection request on the connection established
        // with the network proxy server. This is called right after
        // the connection establishment succeeds.
        //
        void beginWrite(EndPoint endpoint, Buffer buf);
        int endWrite(Buffer buf);

        //
        // Once the connection request has been sent, this is called
        // to prepare and read the response from the proxy server.
        //
        void beginRead(Buffer buf);
        int endRead(Buffer buf);

        //
        // This is called when the response from the proxy has been
        // read. The proxy should copy the extra read data (if any) in the
        // given byte vector.
        //
        void finish(Buffer readBuffer, Buffer writeBuffer);

        //
        // If the proxy host needs to be resolved, this should return
        // a new NetworkProxy containing the IP address of the proxy.
        // This is called from the endpoint host resolver thread, so
        // it's safe if this this method blocks.
        //
        NetworkProxy resolveHost(int protocolSupport);

        //
        // Returns the IP address of the network proxy. This method
        // must not block. It's only called on a network proxy object
        // returned by resolveHost().
        //
        EndPoint getAddress();

        //
        // Returns the name of the proxy, used for tracing purposes.
        //
        string getName();

        //
        // Returns the protocols supported by the proxy.
        //
        int getProtocolSupport();
    }

    public sealed class SOCKSNetworkProxy : NetworkProxy
    {
        public SOCKSNetworkProxy(string host, int port)
        {
            _host = host;
            _port = port;
        }

        private SOCKSNetworkProxy(EndPoint address)
        {
            _address = address;
        }

        public void beginWrite(EndPoint endpoint, Buffer buf)
        {
            if(!(endpoint is IPEndPoint))
            {
                throw new Ice.FeatureNotSupportedException("SOCKS4 does not support domain names");
            }
            else if(endpoint.AddressFamily != AddressFamily.InterNetwork)
            {
                throw new Ice.FeatureNotSupportedException("SOCKS4 only supports IPv4 addresses");
            }

            //
            // SOCKS connect request
            //
            IPEndPoint addr = (IPEndPoint)endpoint;
            buf.resize(9, false);
            ByteBuffer.ByteOrder order = buf.b.order();
            buf.b.order(ByteBuffer.ByteOrder.BIG_ENDIAN); // Network byte order.
            buf.b.position(0);
            buf.b.put(0x04); // SOCKS version 4.
            buf.b.put(0x01); // Command, establish a TCP/IP stream connection
            buf.b.putShort((short)addr.Port); // Port
            buf.b.put(addr.Address.GetAddressBytes()); // IPv4 address
            buf.b.put(0x00); // User ID.
            buf.b.position(0);
            buf.b.limit(buf.size());
            buf.b.order(order);
        }

        public int endWrite(Buffer buf)
        {
            // Once the request is sent, read the response
            return buf.b.hasRemaining() ? SocketOperation.Write : SocketOperation.Read;
        }

        public void beginRead(Buffer buf)
        {
            //
            // Read the SOCKS4 response whose size is 8 bytes.
            //
            buf.resize(8, true);
            buf.b.position(0);
        }

        public int endRead(Buffer buf)
        {
            // We're done once we read the response
            return buf.b.hasRemaining() ? SocketOperation.Read : SocketOperation.None;
        }

        public void finish(Buffer readBuffer, Buffer writeBuffer)
        {
            readBuffer.b.position(0);
            byte b1 = readBuffer.b.get();
            byte b2 = readBuffer.b.get();
            if(b1 != 0x00 || b2 != 0x5a)
            {
                throw new Ice.ConnectFailedException();
            }
        }

        public NetworkProxy resolveHost(int protocolSupport)
        {
            Debug.Assert(_host != null);
            return new SOCKSNetworkProxy(Network.getAddresses(_host,
                                                              _port,
                                                              protocolSupport,
                                                              Ice.EndpointSelectionType.Random,
                                                              false,
                                                              true)[0]);
        }

        public EndPoint getAddress()
        {
            Debug.Assert(_address != null); // Host must be resolved.
            return _address;
        }

        public string getName()
        {
            return "SOCKS";
        }

        public int getProtocolSupport()
        {
            return Network.EnableIPv4;
        }

        private readonly string _host;
        private readonly int _port;
        private readonly EndPoint _address;
    }

    public sealed class HTTPNetworkProxy : NetworkProxy
    {
        public HTTPNetworkProxy(string host, int port)
        {
            _host = host;
            _port = port;
            _protocolSupport = Network.EnableBoth;
        }

        private HTTPNetworkProxy(EndPoint address, int protocolSupport)
        {
            _address = address;
            _protocolSupport = protocolSupport;
        }

        public void beginWrite(EndPoint endpoint, Buffer buf)
        {
            string addr = Network.addrToString(endpoint);
            StringBuilder str = new StringBuilder();
            str.Append("CONNECT ");
            str.Append(addr);
            str.Append(" HTTP/1.1\r\nHost: ");
            str.Append(addr);
            str.Append("\r\n\r\n");
            byte[] b = System.Text.Encoding.ASCII.GetBytes(str.ToString());

            //
            // HTTP connect request
            //
            buf.resize(b.Length, false);
            buf.b.position(0);
            buf.b.put(b);
            buf.b.position(0);
            buf.b.limit(buf.size());
        }

        public int endWrite(Buffer buf)
        {
            // Once the request is sent, read the response
            return buf.b.hasRemaining() ? SocketOperation.Write : SocketOperation.Read;
        }

        public void beginRead(Buffer buf)
        {
            //
            // Read the HTTP response
            //
            buf.resize(7, true); // Enough space for reading at least HTTP1.1
            buf.b.position(0);
        }

        public int endRead(Buffer buf)
        {
            //
            // Check if we received the full HTTP response, if not, continue
            // reading otherwise we're done.
            //
            int end = new HttpParser().isCompleteMessage(buf.b, 0, buf.b.position());
            if(end < 0 && !buf.b.hasRemaining())
            {
                //
                // Read one more byte, we can't easily read bytes in advance
                // since the transport implenentation might be be able to read
                // the data from the memory instead of the socket.
                //
                buf.resize(buf.size() + 1, true);
                return SocketOperation.Read;
            }
            return SocketOperation.None;
        }

        public void finish(Buffer readBuffer, Buffer writeBuffer)
        {
            HttpParser parser = new HttpParser();
            parser.parse(readBuffer.b, 0, readBuffer.b.position());
            if(parser.status() != 200)
            {
                throw new Ice.ConnectFailedException();
            }
        }

        public NetworkProxy resolveHost(int protocolSupport)
        {
            Debug.Assert(_host != null);
            return new HTTPNetworkProxy(Network.getAddresses(_host,
                                                             _port,
                                                             protocolSupport,
                                                             Ice.EndpointSelectionType.Random,
                                                             false,
                                                             true)[0],
                                        protocolSupport);
        }

        public EndPoint getAddress()
        {
            Debug.Assert(_address != null); // Host must be resolved.
            return _address;
        }

        public string getName()
        {
            return "HTTP";
        }

        public int getProtocolSupport()
        {
            return _protocolSupport;
        }

        private readonly string _host;
        private readonly int _port;
        private readonly EndPoint _address;
        private readonly int _protocolSupport;
    }
}
