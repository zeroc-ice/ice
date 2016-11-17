// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import com.zeroc.Ice.EndpointParseException;

final class UdpEndpointI extends IPEndpointI
{
    public UdpEndpointI(ProtocolInstance instance, String ho, int po, java.net.InetSocketAddress sourceAddr,
                        String mcastInterface, int mttl, boolean conn, String conId, boolean co)
    {
        super(instance, ho, po, sourceAddr, conId);
        _mcastInterface = mcastInterface;
        _mcastTtl = mttl;
        _connect = conn;
        _compress = co;
    }

    public UdpEndpointI(ProtocolInstance instance)
    {
        super(instance);
        _connect = false;
        _compress = false;
    }

    public UdpEndpointI(ProtocolInstance instance, com.zeroc.Ice.InputStream s)
    {
        super(instance, s);
        if(s.getEncoding().equals(com.zeroc.Ice.Util.Encoding_1_0))
        {
            s.readByte();
            s.readByte();
            s.readByte();
            s.readByte();
        }
        // Not transmitted.
        //_connect = s.readBool();
        _connect = false;
        _compress = s.readBool();
    }

    //
    // Return the endpoint information.
    //
    @Override
    public com.zeroc.Ice.EndpointInfo getInfo()
    {
        com.zeroc.Ice.UDPEndpointInfo info = new com.zeroc.Ice.UDPEndpointInfo()
        {
            @Override
            public short type()
            {
                return UdpEndpointI.this.type();
            }

            @Override
            public boolean datagram()
            {
                return UdpEndpointI.this.datagram();
            }

            @Override
            public boolean secure()
            {
                return UdpEndpointI.this.secure();
            }
        };
        fillEndpointInfo(info);
        return info;
    }

    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    @Override
    public int timeout()
    {
        return -1;
    }

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    @Override
    public EndpointI timeout(int timeout)
    {
        return this;
    }

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    @Override
    public boolean compress()
    {
        return _compress;
    }

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    @Override
    public EndpointI compress(boolean compress)
    {
        if(compress == _compress)
        {
            return this;
        }
        else
        {
            return new UdpEndpointI(_instance, _host, _port, _sourceAddr, _mcastInterface, _mcastTtl, _connect,
                                    _connectionId, compress);
        }
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    @Override
    public boolean datagram()
    {
        return true;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor.
    //
    @Override
    public Transceiver transceiver()
    {
        return new UdpTransceiver(this, _instance, _host, _port, _mcastInterface, _connect);
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available.
    //
    @Override
    public Acceptor acceptor(String adapterName)
    {
        return null;
    }

    public UdpEndpointI endpoint(UdpTransceiver transceiver)
    {
        return new UdpEndpointI(_instance, _host, transceiver.effectivePort(), _sourceAddr, _mcastInterface,_mcastTtl,
                                _connect, _connectionId, _compress);
    }

    //
    // Convert the endpoint to its string form
    //
    @Override
    public String options()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        String s = super.options();

        if(_mcastInterface.length() != 0)
        {
            s += " --interface " + _mcastInterface;
        }

        if(_mcastTtl != -1)
        {
            s += " --ttl " + _mcastTtl;
        }

        if(_connect)
        {
            s += " -c";
        }

        if(_compress)
        {
            s += " -z";
        }

        return s;
    }

    @Override
    public int compareTo(EndpointI obj) // From java.lang.Comparable
    {
        if(!(obj instanceof UdpEndpointI))
        {
            return type() < obj.type() ? -1 : 1;
        }

        UdpEndpointI p = (UdpEndpointI)obj;
        if(this == p)
        {
            return 0;
        }

        if(!_connect && p._connect)
        {
            return -1;
        }
        else if(!p._connect && _connect)
        {
            return 1;
        }

        if(!_compress && p._compress)
        {
            return -1;
        }
        else if(!p._compress && _compress)
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

        return super.compareTo(obj);
    }

    //
    // Marshal the endpoint
    //
    @Override
    public void streamWriteImpl(com.zeroc.Ice.OutputStream s)
    {
        super.streamWriteImpl(s);
        if(s.getEncoding().equals(com.zeroc.Ice.Util.Encoding_1_0))
        {
            com.zeroc.Ice.Util.Protocol_1_0.ice_writeMembers(s);
            com.zeroc.Ice.Util.Encoding_1_0.ice_writeMembers(s);
        }
        // Not transmitted.
        //s.writeBool(_connect);
        s.writeBool(_compress);
    }

    @Override
    public int hashInit(int h)
    {
        h = super.hashInit(h);
        h = HashUtil.hashAdd(h, _mcastInterface);
        h = HashUtil.hashAdd(h, _mcastTtl);
        h = HashUtil.hashAdd(h, _connect);
        h = HashUtil.hashAdd(h, _compress);
        return h;
    }

    @Override
    public void fillEndpointInfo(com.zeroc.Ice.IPEndpointInfo info)
    {
        super.fillEndpointInfo(info);
        if(info instanceof com.zeroc.Ice.UDPEndpointInfo)
        {
            com.zeroc.Ice.UDPEndpointInfo udpInfo = (com.zeroc.Ice.UDPEndpointInfo)info;
            udpInfo.mcastInterface = _mcastInterface;
            udpInfo.mcastTtl = _mcastTtl;
        }
    }

    @Override
    protected boolean checkOption(String option, String argument, String endpoint)
    {
        if(super.checkOption(option, argument, endpoint))
        {
            return true;
        }

        if(option.equals("-c"))
        {
            if(argument != null)
            {
                throw new EndpointParseException("unexpected argument `" + argument + "' provided for -c option in " +
                                                 endpoint);
            }

            _connect = true;
        }
        else if(option.equals("-z"))
        {
            if(argument != null)
            {
                throw new EndpointParseException("unexpected argument `" + argument + "' provided for -z option in " +
                                                 endpoint);
            }

            _compress = true;
        }
        else if(option.equals("-v") || option.equals("-e"))
        {
            if(argument == null)
            {
                throw new EndpointParseException("no argument provided for " + option + " option in endpoint " +
                                                 endpoint);
            }

            try
            {
                com.zeroc.Ice.EncodingVersion v = com.zeroc.Ice.Util.stringToEncodingVersion(argument);
                if(v.major != 1 || v.minor != 0)
                {
                    _instance.logger().warning("deprecated udp endpoint option: " + option);
                }
            }
            catch(com.zeroc.Ice.VersionParseException e)
            {
                throw new EndpointParseException("invalid version `" + argument + "' in endpoint " + endpoint + ":\n" +
                                                 e.str);
            }
        }
        else if(option.equals("--ttl"))
        {
            if(argument == null)
            {
                throw new EndpointParseException("no argument provided for --ttl option in endpoint " + endpoint);
            }

            try
            {
                _mcastTtl = Integer.parseInt(argument);
            }
            catch(NumberFormatException ex)
            {
                throw new EndpointParseException("invalid TTL value `" + argument + "' in endpoint " + endpoint);
            }

            if(_mcastTtl < 0)
            {
                throw new EndpointParseException("TTL value `" + argument + "' out of range in endpoint " + endpoint);
            }
        }
        else if(option.equals("--interface"))
        {
            if(argument == null)
            {
                throw new EndpointParseException("no argument provided for --interface option in endpoint " + endpoint);
            }
            _mcastInterface = argument;
        }
        else
        {
            return false;
        }
        return true;
    }

    @Override
    protected Connector createConnector(java.net.InetSocketAddress addr, NetworkProxy proxy)
    {
        return new UdpConnector(_instance, addr, _sourceAddr, _mcastInterface, _mcastTtl, _connectionId);
    }

    @Override
    protected IPEndpointI createEndpoint(String host, int port, String connectionId)
    {
        return new UdpEndpointI(_instance, host, port, _sourceAddr, _mcastInterface,_mcastTtl, _connect,
                                connectionId, _compress);
    }

    private String _mcastInterface = "";
    private int _mcastTtl = -1;
    private boolean _connect;
    private boolean _compress;
}
