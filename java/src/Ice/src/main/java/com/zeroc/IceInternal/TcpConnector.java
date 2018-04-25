// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

final class TcpConnector implements Connector
{
    @Override
    public Transceiver connect()
    {
        return new TcpTransceiver(_instance, new StreamSocket(_instance, _proxy, _addr, _sourceAddr));
    }

    @Override
    public short type()
    {
        return _instance.type();
    }

    @Override
    public String toString()
    {
        return Network.addrToString(_proxy == null ? _addr : _proxy.getAddress());
    }

    @Override
    public int hashCode()
    {
        return _hashCode;
    }

    //
    // Only for use by TcpEndpoint
    //
    TcpConnector(ProtocolInstance instance, java.net.InetSocketAddress addr, NetworkProxy proxy,
                 java.net.InetSocketAddress sourceAddr, int timeout, String connectionId)
    {
        _instance = instance;
        _addr = addr;
        _proxy = proxy;
        _sourceAddr = sourceAddr;
        _timeout = timeout;
        _connectionId = connectionId;

        _hashCode = 5381;
        _hashCode = HashUtil.hashAdd(_hashCode , _addr.getAddress().getHostAddress());
        _hashCode = HashUtil.hashAdd(_hashCode , _addr.getPort());
        if(_sourceAddr != null)
        {
            _hashCode = HashUtil.hashAdd(_hashCode , _sourceAddr.getAddress().getHostAddress());
        }
        _hashCode = HashUtil.hashAdd(_hashCode , _timeout);
        _hashCode = HashUtil.hashAdd(_hashCode , _connectionId);
    }

    @Override
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

        if(Network.compareAddress(_sourceAddr, p._sourceAddr) != 0)
        {
            return false;
        }

        return Network.compareAddress(_addr, p._addr) == 0;
    }

    private ProtocolInstance _instance;
    private java.net.InetSocketAddress _addr;
    private NetworkProxy _proxy;
    private java.net.InetSocketAddress _sourceAddr;
    private int _timeout;
    private String _connectionId = "";
    private int _hashCode;
}
