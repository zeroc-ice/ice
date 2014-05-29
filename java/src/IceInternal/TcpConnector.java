// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class TcpConnector implements Connector
{
    public Transceiver connect()
    {
        if(_instance.traceLevel() >= 2)
        {
            String s = "trying to establish " + _instance.protocol() + " connection to " + toString();
            _instance.logger().trace(_instance.traceCategory(), s);
        }

        try
        {
            java.nio.channels.SocketChannel fd = Network.createTcpSocket();
            Network.setBlock(fd, false);
            Network.setTcpBufSize(fd, _instance.properties(), _instance.logger());
            final java.net.InetSocketAddress addr = _proxy != null ? _proxy.getAddress() : _addr;
            Network.doConnect(fd, addr);
            return new TcpTransceiver(_instance, fd, _proxy, _addr);
        }
        catch(Ice.LocalException ex)
        {
            if(_instance.traceLevel() >= 2)
            {
                String s = "failed to establish " + _instance.protocol() + " connection to " + toString() + "\n" + ex;
                _instance.logger().trace(_instance.traceCategory(), s);
            }
            throw ex;
        }
    }

    public short type()
    {
        return _instance.type();
    }

    public String toString()
    {
        return Network.addrToString(_proxy == null ? _addr : _proxy.getAddress());
    }

    public int hashCode()
    {
        return _hashCode;
    }

    //
    // Only for use by TcpEndpoint
    //
    TcpConnector(ProtocolInstance instance, java.net.InetSocketAddress addr, NetworkProxy proxy, int timeout,
                 String connectionId)
    {
        _instance = instance;
        _addr = addr;
        _proxy = proxy;
        _timeout = timeout;
        _connectionId = connectionId;

        _hashCode = 5381;
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _addr.getAddress().getHostAddress());
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _addr.getPort());
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _timeout);
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _connectionId);
    }

    public boolean equals(java.lang.Object obj)
    {
        if(!(obj instanceof TcpConnector))
        {
            return false;
        }

        if(this == obj)
        {
            return true;
        }

        TcpConnector p = (TcpConnector)obj;
        if(_timeout != p._timeout)
        {
            return false;
        }

        if(!_connectionId.equals(p._connectionId))
        {
            return false;
        }

        return Network.compareAddress(_addr, p._addr) == 0;
    } 

    private ProtocolInstance _instance;
    private java.net.InetSocketAddress _addr;
    private NetworkProxy _proxy;
    private int _timeout;
    private String _connectionId = "";
    private int _hashCode;
}
