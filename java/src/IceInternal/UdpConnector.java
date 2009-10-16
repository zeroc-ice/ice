// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
        return new UdpTransceiver(_instance, _endpointInfo, _addr);
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
        return UdpEndpointI.TYPE;
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
    UdpConnector(Instance instance, Ice.UdpEndpointInfo endpointInfo, java.net.InetSocketAddress addr, 
                 String connectionId)
    {
        _instance = instance;
        _endpointInfo = endpointInfo;
        _addr = addr;
        _connectionId = connectionId;

        _hashCode = _addr.getAddress().getHostAddress().hashCode();
        _hashCode = 5 * _hashCode + _addr.getPort();
        _hashCode = 5 * _hashCode + _endpointInfo.mcastInterface.hashCode();
        _hashCode = 5 * _hashCode + _endpointInfo.mcastTtl;
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

        if(_endpointInfo.protocolMajor != p._endpointInfo.protocolMajor)
        {
            return false;
        }

        if(_endpointInfo.protocolMinor != p._endpointInfo.protocolMinor)
        {
            return false;
        }

        if(_endpointInfo.encodingMajor != p._endpointInfo.encodingMajor)
        {
            return false;
        }

        if(_endpointInfo.encodingMinor != p._endpointInfo.encodingMinor)
        {
            return false;
        }

        if(_endpointInfo.mcastTtl != p._endpointInfo.mcastTtl)
        {
            return false;
        }

        if(_endpointInfo.mcastInterface.compareTo(p._endpointInfo.mcastInterface) != 0)
        {
            return false;
        }

        return Network.compareAddress(_addr, p._addr) == 0;
    } 

    private Instance _instance;
    private Ice.UdpEndpointInfo _endpointInfo;
    private java.net.InetSocketAddress _addr;
    private String _connectionId;
    private int _hashCode;
}
