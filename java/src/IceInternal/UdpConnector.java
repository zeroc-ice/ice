// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class UdpConnector implements Connector
{
    public Transceiver
    connect()
    {
        return new UdpTransceiver(_instance, _addr, _mcastInterface, _mcastTtl);
    }

    public java.nio.channels.SelectableChannel
    fd()
    {
        assert(false); // Shouldn't be called, startConnect always completes immediately.
        return null;
    }

    public short
    type()
    {
        return Ice.UDPEndpointType.value;
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
    UdpConnector(Instance instance, java.net.InetSocketAddress addr, String mcastInterface, int mcastTtl, 
                 String connectionId)
    {
        _instance = instance;
        _addr = addr;
        _mcastInterface = mcastInterface;
        _mcastTtl = mcastTtl;
        _connectionId = connectionId;

        _hashCode = 5381;
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _addr.getAddress().getHostAddress());
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _addr.getPort());
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _mcastInterface);
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _mcastTtl);
        _hashCode = IceInternal.HashUtil.hashAdd(_hashCode , _connectionId);
    }

    public boolean
    equals(java.lang.Object obj)
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

        return Network.compareAddress(_addr, p._addr) == 0;
    } 

    private Instance _instance;
    private java.net.InetSocketAddress _addr;
    private String _mcastInterface;
    private int _mcastTtl;
    private String _connectionId;
    private int _hashCode;
}
