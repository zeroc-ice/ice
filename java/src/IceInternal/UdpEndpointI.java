// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class UdpEndpointI extends EndpointI
{
    public
    UdpEndpointI(Instance instance, String ho, int po, String mif, int mttl, Ice.ProtocolVersion p, 
                 Ice.EncodingVersion e, boolean conn, String conId, boolean co)
    {
        super(p, e, conId);
        _instance = instance;
        _host = ho;
        _port = po;
        _mcastInterface = mif;
        _mcastTtl = mttl;
        _connect = conn;
        _compress = co;
        calcHashValue();
    }

    public
    UdpEndpointI(Instance instance, String str, boolean oaEndpoint)
    {
        super(Protocol.currentProtocol, instance.defaultsAndOverrides().defaultEncoding, "");
        _instance = instance;
        _host = null;
        _port = 0;
        _connect = false;
        _compress = false;

        String[] arr = str.split("[ \t\n\r]+");

        int i = 0;
        while(i < arr.length)
        {
            if(arr[i].length() == 0)
            {
                i++;
                continue;
            }

            String option = arr[i++];
            if(option.charAt(0) != '-')
            {
                throw new Ice.EndpointParseException("expected an endpoint option but found `" + option +
                                                     "' in endpoint `udp " + str + "'");
            }

            String argument = null;
            if(i < arr.length && arr[i].charAt(0) != '-')
            {
                argument = arr[i++];
                if(argument.charAt(0) == '\"' && argument.charAt(argument.length() - 1) == '\"')
                {
                    argument = argument.substring(1, argument.length() - 1);
                }
            }

            if(option.equals("-h"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for -h option in endpoint `udp "
                                                         + str + "'");
                }

                _host = argument;
            }
            else if(option.equals("-p"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for -p option in endpoint `udp "
                                                         + str + "'");
                }

                try
                {
                    _port = Integer.parseInt(argument);
                }
                catch(NumberFormatException ex)
                {
                    throw new Ice.EndpointParseException("invalid port value `" + argument + "' in endpoint `udp " +
                                                         str + "'");
                }

                if(_port < 0 || _port > 65535)
                {
                    throw new Ice.EndpointParseException("port value `" + argument +
                                                         "' out of range in endpoint `udp " + str + "'");
                }
            }
            else if(option.equals("-c"))
            {
                if(argument != null)
                {
                    throw new Ice.EndpointParseException("unexpected argument `" + argument +
                                                         "' provided for -c option in `udp " + str + "'");
                }

                _connect = true;
                break;
            }
            else if(option.equals("-z"))
            {
                if(argument != null)
                {
                    throw new Ice.EndpointParseException("unexpected argument `" + argument +
                                                         "' provided for -z option in `udp " + str + "'");
                }

                _compress = true;
                break;
            }
            else if(option.equals("--interface"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for --interface option in endpoint `udp "
                                                         + str + "'");
                }

                _mcastInterface = argument;
            }
            else if(option.equals("--ttl"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("no argument provided for --ttl option in endpoint `udp "
                                                         + str + "'");
                }

                try
                {
                    _mcastTtl = Integer.parseInt(argument);
                }
                catch(NumberFormatException ex)
                {
                    throw new Ice.EndpointParseException("invalid TTL value `" + argument + "' in endpoint `udp " +
                                                         str + "'");
                }

                if(_mcastTtl < 0)
                {
                    throw new Ice.EndpointParseException("TTL value `" + argument +
                                                         "' out of range in endpoint `udp " + str + "'");
                }
            }
            else
            {
                parseOption(option, argument, "udp", str);
            }
        }

        if(_host == null)
        {
            _host = _instance.defaultsAndOverrides().defaultHost;
        }
        else if(_host.equals("*"))
        {
            if(oaEndpoint)
            {
                _host = null;
            }
            else
            {
                throw new Ice.EndpointParseException("`-h *' not valid for proxy endpoint `udp " + str + "'");
            }
        }

        if(_host == null)
        {
            _host = "";
        }

        calcHashValue();
    }

    public
    UdpEndpointI(BasicStream s)
    {
        super(new Ice.ProtocolVersion(), new Ice.EncodingVersion(), "");
        _instance = s.instance();
        s.startReadEncaps();
        _host = s.readString();
        _port = s.readInt();
        _protocol.__read(s);
        _encoding.__read(s);
        // Not transmitted.
        //_connect = s.readBool();
        _connect = false;
        _compress = s.readBool();
        s.endReadEncaps();
        calcHashValue();
    }

    //
    // Marshal the endpoint
    //
    public void
    streamWrite(BasicStream s)
    {
        s.writeShort(Ice.UDPEndpointType.value);
        s.startWriteEncaps();
        s.writeString(_host);
        s.writeInt(_port);
        _protocol.__write(s);
        _encoding.__write(s);
        // Not transmitted.
        //s.writeBool(_connect);
        s.writeBool(_compress);
        s.endWriteEncaps();
    }

    //
    // Convert the endpoint to its string form
    //
    public String
    _toString()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        String s = "udp";

        if(!_protocol.equals(Ice.Util.Protocol_1_0))
        {
            s += " -v " + Ice.Util.protocolVersionToString(_protocol);
        }
        
        if(!_encoding.equals(Ice.Util.Encoding_1_0))
        {
            s += " -e " + Ice.Util.encodingVersionToString(_encoding);
        }

        if(_host != null && _host.length() > 0)
        {
            s += " -h "; 
            boolean addQuote = _host.indexOf(':') != -1;
            if(addQuote)
            {
                s += "\"";
            }
            s += _host; 
            if(addQuote)
            {
                s += "\"";
            }
        }

        s += " -p " + _port;

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

    //
    // Return the endpoint information.
    //
    public Ice.EndpointInfo
    getInfo()
    {
        return new Ice.UDPEndpointInfo(_protocol, _encoding, -1, _compress, _host, _port, _mcastInterface, _mcastTtl)
            {
                public short type()
                {
                    return Ice.UDPEndpointType.value;
                }
                
                public boolean datagram()
                {
                    return true;
                }
                
                public boolean secure()
                {
                    return false;
                }
        };
    }

    //
    // Return the endpoint type
    //
    public short
    type()
    {
        return Ice.UDPEndpointType.value;
    }

    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    public int
    timeout()
    {
        return -1;
    }

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    public boolean
    compress()
    {
        return _compress;
    }

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    public EndpointI
    compress(boolean compress)
    {
        if(compress == _compress)
        {
            return this;
        }
        else
        {
            return new UdpEndpointI(_instance, _host, _port, _mcastInterface, _mcastTtl, _protocol, _encoding, 
                                    _connect, _connectionId, compress);
        }
    }

    //
    // Return a new endpoint with a different connection id.
    //
    public EndpointI
    connectionId(String connectionId)
    {
        if(connectionId.equals(_connectionId))
        {
            return this;
        }
        else
        {
            return new UdpEndpointI(_instance, _host, _port, _mcastInterface, _mcastTtl, _protocol, _encoding,
                                    _connect, connectionId, _compress);
        }
    }

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    public EndpointI
    timeout(int timeout)
    {
        return this;
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    public boolean
    datagram()
    {
        return true;
    }

    //
    // Return true if the endpoint is secure.
    //
    public boolean
    secure()
    {
        return false;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public Transceiver
    transceiver(EndpointIHolder endpoint)
    {
        UdpTransceiver p = new UdpTransceiver(_instance, _host, _port, _mcastInterface, _connect);
        endpoint.value = new UdpEndpointI(_instance, _host, p.effectivePort(), _mcastInterface, _mcastTtl, 
                                          _protocol, _encoding, _connect, _connectionId, _compress);
        return p;
    }

    //
    // Return connectors for this endpoint, or empty list if no connector
    // is available.
    //
    public java.util.List<Connector>
    connectors()
    {
        return connectors(Network.getAddresses(_host, _port, _instance.protocolSupport()));
    }

    public void
    connectors_async(EndpointI_connectors callback)
    {
        _instance.endpointHostResolver().resolve(_host, _port, this, callback);
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    public Acceptor
    acceptor(EndpointIHolder endpoint, String adapterName)
    {
        endpoint.value = this;
        return null;
    }

    //
    // Expand endpoint out in to separate endpoints for each local
    // host if listening on INADDR_ANY.
    //
    public java.util.List<EndpointI>
    expand()
    {
        java.util.ArrayList<EndpointI> endps = new java.util.ArrayList<EndpointI>();
        java.util.ArrayList<String> hosts =
            Network.getHostsForEndpointExpand(_host, _instance.protocolSupport(), false);
        if(hosts == null || hosts.isEmpty())
        {
            endps.add(this);
        }
        else
        {
            for(String host : hosts)
            {
                endps.add(new UdpEndpointI(_instance, host, _port, _mcastInterface, _mcastTtl,
                                           _protocol, _encoding, _connect, _connectionId, _compress));
            }
        }
        return endps;
    }

    //
    // Check whether the endpoint is equivalent to another one.
    //
    public boolean
    equivalent(EndpointI endpoint)
    {
        if(!(endpoint instanceof UdpEndpointI))
        {
            return false;
        }
        UdpEndpointI udpEndpointI = (UdpEndpointI)endpoint;
        return udpEndpointI._host.equals(_host) && udpEndpointI._port == _port;
    }

    public int
    hashCode()
    {
        return _hashCode;
    }

    public int
    compareTo(EndpointI obj) // From java.lang.Comparable
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
        else
        {
            int r = super.compareTo(p);
            if(r != 0)
            {
                return r;
            }
        }

        if(_port < p._port)
        {
            return -1;
        }
        else if(p._port < _port)
        {
            return 1;
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

        return _host.compareTo(p._host);
    }

    public java.util.List<Connector>
    connectors(java.util.List<java.net.InetSocketAddress> addresses)
    {
        java.util.ArrayList<Connector> connectors = new java.util.ArrayList<Connector>();
        for(java.net.InetSocketAddress p : addresses)
        {
            connectors.add(new UdpConnector(_instance, p, _mcastInterface, _mcastTtl, _protocol, _encoding,
                                            _connectionId));
        }
        return connectors;
    }

    private void
    calcHashValue()
    {
        int h = 5381;
        h = IceInternal.HashUtil.hashAdd(h, Ice.UDPEndpointType.value);
        h = IceInternal.HashUtil.hashAdd(h, _host);
        h = IceInternal.HashUtil.hashAdd(h, _port);
        h = IceInternal.HashUtil.hashAdd(h, _mcastInterface);
        h = IceInternal.HashUtil.hashAdd(h, _mcastTtl);
        h = IceInternal.HashUtil.hashAdd(h, _connect);
        h = IceInternal.HashUtil.hashAdd(h, _protocol);
        h = IceInternal.HashUtil.hashAdd(h, _encoding);
        h = IceInternal.HashUtil.hashAdd(h, _connectionId);
        h = IceInternal.HashUtil.hashAdd(h, _compress);
        _hashCode = h;
    }

    private Instance _instance;
    private String _host;
    private int _port;
    private String _mcastInterface = "";
    private int _mcastTtl = -1;
    private boolean _connect;
    private boolean _compress;
    private int _hashCode;
}
