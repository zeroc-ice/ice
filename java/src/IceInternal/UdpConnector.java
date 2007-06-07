// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class UdpConnector implements Connector, java.lang.Comparable
{
    final static short TYPE = 3;

    public Transceiver
    connect(int timeout)
    {
        return new UdpTransceiver(_instance, _addr, _mcastInterface, _mcastTtl);
    }

    public short
    type()
    {
        return TYPE;
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
        _traceLevels = instance.traceLevels();
        _logger = instance.initializationData().logger;
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

    //
    // Compare connectors for sorting purposes
    //
    public boolean
    equals(java.lang.Object obj)
    {
        return compareTo(obj) == 0;
    }

    public int
    compareTo(java.lang.Object obj) // From java.lang.Comparable
    {
        UdpConnector p = null;

        try
        {
            p = (UdpConnector)obj;
        }
        catch(ClassCastException ex)
        {
            try
            {
                Connector c = (Connector)obj;
                return type() < c.type() ? -1 : 1;
            }
            catch(ClassCastException ee)
            {
                assert(false);
            }
        }

        if(this == p)
        {
            return 0;
        }

        if(!_connectionId.equals(p._connectionId))
        {
            return _connectionId.compareTo(p._connectionId);
        }

        if(_protocolMajor < p._protocolMajor)
        {
            return -1;
        }
        else if(p._protocolMajor < _protocolMajor)
        {
            return 1;
        }

        if(_protocolMinor < p._protocolMinor)
        {
            return -1;
        }
        else if(p._protocolMinor < _protocolMinor)
        {
            return 1;
        }

        if(_encodingMajor < p._encodingMajor)
        {
            return -1;
        }
        else if(p._encodingMajor < _encodingMajor)
        {
            return 1;
        }

        if(_encodingMinor < p._encodingMinor)
        {
            return -1;
        }
        else if(p._encodingMinor < _encodingMinor)
        {
            return 1;
        }

        if(_mcastTtl < p._mcastTtl)
        {
            return -1;
        }
        else if(p._mcastTtl < _mcastTtl)
        {
            return 1;
        }

        int rc = _mcastInterface.compareTo(p._mcastInterface);
        if(rc != 0)
        {
            return rc;
        }

        return Network.compareAddress(_addr, p._addr);
    } 

    private Instance _instance;
    private TraceLevels _traceLevels;
    private Ice.Logger _logger;
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
