// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Net;
using System.Net.Sockets;

namespace Ice.Internal;

internal sealed class TcpTransceiver : Transceiver
{
    public bool isWaitingToBeRead => _stream.isWaitingToBeRead;

    public Socket fd()
    {
        return _stream.fd();
    }

    public int initialize(Buffer readBuffer, Buffer writeBuffer, ref bool hasMoreData)
    {
        return _stream.connect(readBuffer, writeBuffer, ref hasMoreData);
    }

    public int closing(bool initiator, Ice.LocalException ex)
    {
        // If we are initiating the connection closure, wait for the peer
        // to close the TCP/IP connection. Otherwise, close immediately.
        return initiator ? SocketOperation.Read : SocketOperation.None;
    }

    public void close()
    {
        _stream.close();
    }

    public EndpointI bind()
    {
        Debug.Assert(false);
        return null;
    }

    public void destroy()
    {
        _stream.destroy();
    }

    public int write(Buffer buf)
    {
        return _stream.write(buf);
    }

    public int read(Buffer buf, ref bool hasMoreData)
    {
        return _stream.read(buf);
    }

    public bool startRead(Buffer buf, AsyncCallback callback, object state)
    {
        return _stream.startRead(buf, callback, state);
    }

    public void finishRead(Buffer buf)
    {
        _stream.finishRead(buf);
    }

    public bool startWrite(Buffer buf, AsyncCallback callback, object state, out bool completed)
    {
        return _stream.startWrite(buf, callback, state, out completed);
    }

    public void finishWrite(Buffer buf)
    {
        _stream.finishWrite(buf);
    }

    public string protocol()
    {
        return _instance.protocol();
    }

    public Ice.ConnectionInfo getInfo()
    {
        Ice.TCPConnectionInfo info = new Ice.TCPConnectionInfo();
        if (_stream.fd() != null)
        {
            EndPoint localEndpoint = Network.getLocalAddress(_stream.fd());
            info.localAddress = Network.endpointAddressToString(localEndpoint);
            info.localPort = Network.endpointPort(localEndpoint);
            EndPoint remoteEndpoint = Network.getRemoteAddress(_stream.fd());
            info.remoteAddress = Network.endpointAddressToString(remoteEndpoint);
            info.remotePort = Network.endpointPort(remoteEndpoint);
            info.rcvSize = Network.getRecvBufferSize(_stream.fd());
            info.sndSize = Network.getSendBufferSize(_stream.fd());
        }
        return info;
    }

    public void checkSendSize(Buffer buf)
    {
    }

    public void setBufferSize(int rcvSize, int sndSize)
    {
        _stream.setBufferSize(rcvSize, sndSize);
    }

    public override string ToString()
    {
        return _stream.ToString();
    }

    public string toDetailedString()
    {
        return ToString();
    }

    //
    // Only for use by TcpConnector, TcpAcceptor
    //
    internal TcpTransceiver(ProtocolInstance instance, StreamSocket stream)
    {
        _instance = instance;
        _stream = stream;
    }

    private readonly ProtocolInstance _instance;
    private readonly StreamSocket _stream;
}
