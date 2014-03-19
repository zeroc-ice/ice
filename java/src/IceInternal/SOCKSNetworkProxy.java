// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class SOCKSNetworkProxy implements NetworkProxy
{
    public SOCKSNetworkProxy(String host, int port)
    {
        _host = host;
        _port = port;
    }

    private SOCKSNetworkProxy(java.net.InetSocketAddress address)
    {
        _address = address;
    }

    public void beginWriteConnectRequest(java.net.InetSocketAddress endpoint, Buffer buf)
    {
        final java.net.InetAddress addr = endpoint.getAddress();
        if(addr == null)
        {
            throw new Ice.FeatureNotSupportedException("SOCKS4 does not support domain names");
        }
        else if(!(addr instanceof java.net.Inet4Address))
        {
            throw new Ice.FeatureNotSupportedException("SOCKS4 only supports IPv4 addresses");
        }

        //
        // SOCKS connect request
        //
        buf.resize(9, false);
        final java.nio.ByteOrder order = buf.b.order();
        buf.b.order(java.nio.ByteOrder.BIG_ENDIAN); // Network byte order.
        buf.b.position(0);
        buf.b.put((byte)0x04); // SOCKS version 4.
        buf.b.put((byte)0x01); // Command, establish a TCP/IP stream connection
        buf.b.putShort((short)endpoint.getPort()); // Port
        buf.b.put(addr.getAddress()); // IPv4 address
        buf.b.put((byte)0x00); // User ID.
        buf.b.position(0);
        buf.b.limit(buf.size());
        buf.b.order(order);
    }

    public void endWriteConnectRequest(Buffer buf)
    {
        buf.reset();
    }

    public void beginReadConnectRequestResponse(Buffer buf)
    {
        //
        // Read the SOCKS4 response whose size is 8 bytes.
        //
        buf.resize(8, true);
        buf.b.position(0);
    }

    public void endReadConnectRequestResponse(Buffer buf)
    {
        buf.b.position(0);
        byte b1 = buf.b.get();
        byte b2 = buf.b.get();
        if(b1 != 0x00 || b2 != 0x5a)
        {
            throw new Ice.ConnectFailedException();
        }
        buf.reset();
    }

    public NetworkProxy resolveHost()
    {
        assert(_host != null);
        return new SOCKSNetworkProxy(Network.getAddresses(_host,
                                                          _port,
                                                          Network.EnableIPv4,
                                                          Ice.EndpointSelectionType.Random,
                                                          false).get(0));
    }

    public java.net.InetSocketAddress getAddress()
    {
        assert(_address != null); // Host must be resolved.
        return _address;
    }

    public String getName()
    {
        return "SOCKS";
    }

    private String _host;
    private int _port;
    private java.net.InetSocketAddress _address;
}
