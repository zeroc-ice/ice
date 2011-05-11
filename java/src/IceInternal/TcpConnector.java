// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class TcpConnector implements Connector
{
    public Transceiver
    connect()
    {
        if(_traceLevels.network >= 2)
        {
            String s = "trying to establish tcp connection to " + toString();
            _logger.trace(_traceLevels.networkCat, s);
        }

        try
        {
            java.nio.channels.SocketChannel fd = Network.createTcpSocket();
            Network.setBlock(fd, false);
            Network.setTcpBufSize(fd, _instance.initializationData().properties, _logger);
            boolean connected = Network.doConnect(fd, _addr);
            if(connected)
            {
                if(_traceLevels.network >= 1)
                {
                    String s = "tcp connection established\n" + Network.fdToString(fd);
                    _logger.trace(_traceLevels.networkCat, s);
                }
            }
            return new TcpTransceiver(_instance, fd, connected);
        }
        catch(Ice.LocalException ex)
        {
            if(_traceLevels.network >= 2)
            {
                String s = "failed to establish tcp connection to " + toString() + "\n" + ex;
                _logger.trace(_traceLevels.networkCat, s);
            }
            throw ex;
        }
    }

    public short
    type()
    {
        return Ice.TCPEndpointType.value;
    }

    public String
    toString()
    {
        return Network.addrToString(_addr);
    }

    public int
    hashCode()
    {
        return _hashCode;
    }

    //
    // Only for use by TcpEndpoint
    //
    TcpConnector(Instance instance, java.net.InetSocketAddress addr, int timeout, String connectionId)
    {
        _instance = instance;
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;
        _addr = addr;
        _timeout = timeout;
        _connectionId = connectionId;

        _hashCode = _addr.getAddress().getHostAddress().hashCode();
        _hashCode = 5 * _hashCode + _addr.getPort();
        _hashCode = 5 * _hashCode + _timeout;
        _hashCode = 5 * _hashCode + _connectionId.hashCode();
    }

    public boolean
    equals(java.lang.Object obj)
    {
        TcpConnector p = null;

        try
        {
            p = (TcpConnector)obj;
        }
        catch(ClassCastException ex)
        {
            return false;
        }

        if(this == p)
        {
            return true;
        }

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

    private Instance _instance;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
    private java.net.InetSocketAddress _addr;
    private int _timeout;
    private String _connectionId = "";
    private int _hashCode;
}
