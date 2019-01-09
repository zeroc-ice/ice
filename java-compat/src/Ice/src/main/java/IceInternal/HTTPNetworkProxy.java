// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

public final class HTTPNetworkProxy implements NetworkProxy
{
    public HTTPNetworkProxy(String host, int port)
    {
        _host = host;
        _port = port;
        _protocolSupport = Network.EnableBoth;
    }

    private HTTPNetworkProxy(java.net.InetSocketAddress address, int protocolSupport)
    {
        _address = address;
        _protocolSupport = protocolSupport;
    }

    @Override
    public void beginWrite(java.net.InetSocketAddress endpoint, Buffer buf)
    {
        String addr = Network.addrToString(endpoint);
        StringBuilder str = new StringBuilder();
        str.append("CONNECT ");
        str.append(addr);
        str.append(" HTTP/1.1\r\nHost: ");
        str.append(addr);
        str.append("\r\n\r\n");

        byte[] b = str.toString().getBytes(java.nio.charset.StandardCharsets.US_ASCII);

        //
        // HTTP connect request
        //
        buf.resize(b.length, false);
        buf.position(0);
        buf.b.put(b);
        buf.position(0);
        buf.limit(buf.size());
    }

    @Override
    public int endWrite(Buffer buf)
    {
        // Once the request is sent, read the response
        return buf.b.hasRemaining() ? SocketOperation.Write : SocketOperation.Read;
    }

    @Override
    public void beginRead(Buffer buf)
    {
        //
        // Read the HTTP response
        //
        buf.resize(7, true); // Enough space for reading at least HTTP1.1
        buf.position(0);
    }

    @Override
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

    @Override
    public void finish(Buffer readBuffer, Buffer writeBuffer)
    {
        HttpParser parser = new HttpParser();
        parser.parse(readBuffer.b, 0, readBuffer.b.position());
        if(parser.status() != 200)
        {
            throw new Ice.ConnectFailedException();
        }
    }

    @Override
    public NetworkProxy resolveHost(int protocol)
    {
        assert(_host != null);
        return new HTTPNetworkProxy(Network.getAddresses(_host,
                                                         _port,
                                                         protocol,
                                                         Ice.EndpointSelectionType.Random,
                                                         false,
                                                         true).get(0),
                                    protocol);
    }

    @Override
    public java.net.InetSocketAddress getAddress()
    {
        assert(_address != null); // Host must be resolved.
        return _address;
    }

    @Override
    public String getName()
    {
        return "HTTP";
    }

    @Override
    public int getProtocolSupport()
    {
        return _protocolSupport;
    }

    private String _host;
    private int _port;
    private java.net.InetSocketAddress _address;
    private int _protocolSupport;
}
