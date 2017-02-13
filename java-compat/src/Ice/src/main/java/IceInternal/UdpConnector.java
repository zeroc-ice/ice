// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class UdpConnector implements Connector
{
    @Override
    public Transceiver connect()
    {
        return new UdpTransceiver(_instance, _addr, _sourceAddr, _mcastInterface, _mcastTtl);
    }

    public java.nio.channels.SelectableChannel fd()
    {
        assert(false); // Shouldn't be called, startConnect always completes immediately.
        return null;
    }

    @Override
    public short type()
    {
        return _instance.type();
    }

    @Override
    public String toString()
    {
        return Network.addrToString(_addr);
    }

    @Override
    public int hashCode()
    {
        return _hashCode;
    }

    //
    // Only for use by UdpEndpointI
    //
    UdpConnector(ProtocolInstance instance, java.net.InetSocketAddress addr, java.net.InetSocketAddress sourceAddr,
                 String mcastInterface, int mcastTtl, String connectionId)
    {
        _instance = instance;
        _addr = addr;
        _sourceAddr = sourceAddr;
        _mcastInterface = mcastInterface;
        _mcastTtl = mcastTtl;
        _connectionId = connectionId;

        _hashCode = 5381;
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _addr.getAddress().getHostAddress());
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _addr.getPort());
        if(_sourceAddr != null)
        {
            _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _sourceAddr.getAddress().getHostAddress());
        }
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _mcastInterface);
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _mcastTtl);
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _connectionId);
    }

    @Override
    public boolean equals(java.lang.Object obj)
    {
        if(!(obj instanceof UdpConnector))
        {
            return false;
        }

        if(this == obj)
        {
            return true;
        }

        UdpConnector p = (UdpConnector)obj;
        if(!_connectionId.equals(p._connectionId))
        {
            return false;
        }

        if(_mcastTtl != p._mcastTtl)
        {
            return false;
        }

        if(_mcastInterface.compareTo(p._mcastInterface) != 0)
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
    private java.net.InetSocketAddress _sourceAddr;
    private String _mcastInterface;
    private int _mcastTtl;
    private String _connectionId;
    private int _hashCode;
}
