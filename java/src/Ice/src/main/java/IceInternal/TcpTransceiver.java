// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class TcpTransceiver implements Transceiver, WSTransceiverDelegate
{
    @Override
    public java.nio.channels.SelectableChannel fd()
    {
        assert(_stream != null);
        return _stream.fd();
    }

    @Override
    public int initialize(Buffer readBuffer, Buffer writeBuffer, Ice.Holder<Boolean> moreData)
    {
        return _stream.connect(readBuffer, writeBuffer);
    }

    @Override
    public int closing(boolean initiator, Ice.LocalException ex)
    {
        // If we are initiating the connection closure, wait for the peer
        // to close the TCP/IP connection. Otherwise, close immediately.
        return initiator ? SocketOperation.Read : SocketOperation.None;
    }

    @Override
    public void close()
    {
        _stream.close();
    }

    @Override
    public EndpointI bind()
    {
        assert(false);
        return null;
    }

    @Override
    public int write(Buffer buf)
    {
        return _stream.write(buf);
    }

    @Override
    public int read(Buffer buf, Ice.Holder<Boolean> moreData)
    {
        return _stream.read(buf);
    }

    @Override
    public String protocol()
    {
        return _instance.protocol();
    }

    @Override
    public String toString()
    {
        return _stream.toString();
    }

    @Override
    public String toDetailedString()
    {
        return toString();
    }

    @Override
    public Ice.ConnectionInfo getInfo()
    {
        Ice.TCPConnectionInfo info = new Ice.TCPConnectionInfo();
        fillConnectionInfo(info);
        return info;
    }

    @Override
    public Ice.ConnectionInfo getWSInfo(java.util.Map<String, String> headers)
    {
        Ice.WSConnectionInfo info = new Ice.WSConnectionInfo();
        fillConnectionInfo(info);
        info.headers = headers; // Provided header is a copy so no need to clone here.
        return info;
    }

    @Override
    public void checkSendSize(Buffer buf)
    {
    }

    @Override
    public void setBufferSize(int rcvSize, int sndSize)
    {
        _stream.setBufferSize(rcvSize, sndSize);
    }

    TcpTransceiver(ProtocolInstance instance, StreamSocket stream)
    {
        _instance = instance;
        _stream = stream;
    }

    private void fillConnectionInfo(Ice.IPConnectionInfo info)
    {
        if(_stream.fd() != null)
        {
            java.net.Socket socket = _stream.fd().socket();
            info.localAddress = socket.getLocalAddress().getHostAddress();
            info.localPort = socket.getLocalPort();
            if(socket.getInetAddress() != null)
            {
                info.remoteAddress = socket.getInetAddress().getHostAddress();
                info.remotePort = socket.getPort();
            }
            if(!socket.isClosed())
            {
                info.rcvSize = Network.getRecvBufferSize(_stream.fd());
                info.sndSize = Network.getSendBufferSize(_stream.fd());
            }
        }
    }

    final private ProtocolInstance _instance;
    final private StreamSocket _stream;
}
