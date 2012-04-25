// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
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
                 byte protocolMajor, byte protocolMinor, byte encodingMajor, byte encodingMinor, String connectionId)
    {
        _instance = instance;
        _addr = addr;
        _mcastInterface = mcastInterface;
        _mcastTtl = mcastTtl;
        _protocolMajor = protocolMajor;
        _protocolMinor = protocolMinor;
        _encodingMajor = encodingMajor;
        _encodingMinor = encodingMinor;
        _connectionId = connectionId;

        _hashCode = _addr.getAddress().getHostAddress().hashCode();
        _hashCode = 5 * _hashCode + _addr.getPort();
        _hashCode = 5 * _hashCode + _mcastInterface.hashCode();
        _hashCode = 5 * _hashCode + _mcastTtl;
        _hashCode = 5 * _hashCode + _connectionId.hashCode();
    }

    public boolean
    equals(java.lang.Object obj)
    {
        UdpConnector p = null;

        try
        {
            p = (UdpConnector)obj;
        }
        catch(ClassCastException ex)
        {
            return false;
        }

        if(this == p)
        {
            return true;
        }

        if(!_connectionId.equals(p._connectionId))
        {
            return false;
        }

        if(_protocolMajor != p._protocolMajor)
        {
            return false;
        }

        if(_protocolMinor != p._protocolMinor)
        {
            return false;
        }

        if(_encodingMajor != p._encodingMajor)
        {
            return false;
        }

        if(_encodingMinor != p._encodingMinor)
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
    private byte _protocolMajor;
    private byte _protocolMinor;
    private byte _encodingMajor;
    private byte _encodingMinor;
    private String _connectionId;
    private int _hashCode;
}
