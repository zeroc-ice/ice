// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class TcpEndpointI extends EndpointI
{
    public
    TcpEndpointI(Instance instance, String ho, int po, int ti, Ice.ProtocolVersion pv, Ice.EncodingVersion ev,
                 String conId, boolean co)
    {
        super(pv, ev);
        _instance = instance;
        _host = ho;
        _port = po;
        _timeout = ti;
        _connectionId = conId;
        _compress = co;
        calcHashValue();
    }

    public
    TcpEndpointI(Instance instance, String str, boolean oaEndpoint)
    {
        super(Protocol.currentProtocol, instance.defaultsAndOverrides().defaultEncoding);
        _instance = instance;
        _host = null;
        _port = 0;
        _timeout = -1;
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
            if(option.length() != 2 || option.charAt(0) != '-')
            {
                throw new Ice.EndpointParseException("expected an endpoint option but found `" + option +
                                                     "' in endpoint `tcp " + str + "'");
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

            switch(option.charAt(1))
            {
                case 'h':
                {
                    if(argument == null)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -h option in endpoint `tcp "
                                                             + str + "'");
                    }

                    _host = argument;
                    break;
                }

                case 'p':
                {
                    if(argument == null)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -p option in endpoint `tcp "
                                                             + str + "'");
                    }

                    try
                    {
                        _port = Integer.parseInt(argument);
                    }
                    catch(NumberFormatException ex)
                    {
                        throw new Ice.EndpointParseException("invalid port value `" + argument +
                                                             "' in endpoint `tcp " + str + "'");
                    }

                    if(_port < 0 || _port > 65535)
                    {
                        throw new Ice.EndpointParseException("port value `" + argument +
                                                             "' out of range in endpoint `tcp " + str + "'");
                    }

                    break;
                }

                case 't':
                {
                    if(argument == null)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -t option in endpoint `tcp "
                                                             + str + "'");
                    }

                    try
                    {
                        _timeout = Integer.parseInt(argument);
                    }
                    catch(NumberFormatException ex)
                    {
                        throw new Ice.EndpointParseException("invalid timeout value `" + argument +
                                                             "' in endpoint `tcp " + str + "'");
                    }

                    break;
                }

                case 'z':
                {
                    if(argument != null)
                    {
                        throw new Ice.EndpointParseException("unexpected argument `" + argument +
                                                             "' provided for -z option in `tcp " + str + "'");
                    }

                    _compress = true;
                    break;
                }

                default:
                {
                    parseOption(option, argument, "tcp", str);
                    break;
                }
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
                throw new Ice.EndpointParseException("`-h *' not valid for proxy endpoint `tcp " + str + "'");
            }
        }

        if(_host == null)
        {
            _host = "";
        }

        calcHashValue();
    }

    public
    TcpEndpointI(BasicStream s)
    {
        super(new Ice.ProtocolVersion(), new Ice.EncodingVersion());
        _instance = s.instance();
        s.startReadEncaps();
        _host = s.readString();
        _port = s.readInt();
        _timeout = s.readInt();
        _compress = s.readBool();
        if(!s.getReadEncoding().equals(Ice.Util.Encoding_1_0))
        {
            _protocol = new Ice.ProtocolVersion();
            _protocol.__read(s);
            _encoding = new Ice.EncodingVersion();
            _encoding.__read(s);
        }
        else
        {
            _protocol = Ice.Util.Protocol_1_0;
            _encoding = Ice.Util.Encoding_1_0;
        }
        s.endReadEncaps();
        calcHashValue();
    }

    //
    // Marshal the endpoint
    //
    public void
    streamWrite(BasicStream s)
    {
        s.writeShort(Ice.TCPEndpointType.value);
        s.startWriteEncaps();
        s.writeString(_host);
        s.writeInt(_port);
        s.writeInt(_timeout);
        s.writeBool(_compress);
        if(!s.getWriteEncoding().equals(Ice.Util.Encoding_1_0))
        {
            _protocol.__write(s);
            _encoding.__write(s);
        }
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
        String s = "tcp";

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

        if(_timeout != -1)
        {
            s += " -t " + _timeout;
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
        return new Ice.TCPEndpointInfo(_protocol, _encoding, _timeout, _compress, _host, _port)
            {
                public short type()
                {
                    return Ice.TCPEndpointType.value;
                }
                
                public boolean datagram()
                {
                    return false;
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
        return Ice.TCPEndpointType.value;
    }

    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    public int
    timeout()
    {
        return _timeout;
    }

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    public EndpointI
    timeout(int timeout)
    {
        if(timeout == _timeout)
        {
            return this;
        }
        else
        {
            return new TcpEndpointI(_instance, _host, _port, timeout, _protocol, _encoding, _connectionId, _compress);
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
            return new TcpEndpointI(_instance, _host, _port, _timeout, _protocol, _encoding, connectionId, _compress);
        }
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
            return new TcpEndpointI(_instance, _host, _port, _timeout, _protocol, _encoding, _connectionId, compress);
        }
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    public boolean
    datagram()
    {
        return false;
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
        endpoint.value = this;
        return null;
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
        TcpAcceptor p = new TcpAcceptor(_instance, _host, _port);
        endpoint.value = new TcpEndpointI(_instance, _host, p.effectivePort(), _timeout, _protocol, _encoding, 
                                          _connectionId, _compress);
        return p;
    }

    //
    // Expand endpoint out in to separate endpoints for each local
    // host if listening on INADDR_ANY.
    //
    public java.util.List<EndpointI>
    expand()
    {
        java.util.List<EndpointI> endps = new java.util.ArrayList<EndpointI>();
        java.util.List<String> hosts = Network.getHostsForEndpointExpand(_host, _instance.protocolSupport(), false);
        if(hosts == null || hosts.isEmpty())
        {
            endps.add(this);
        }
        else
        {
            for(String h : hosts)
            {
                endps.add(new TcpEndpointI(_instance, h, _port, _timeout, _protocol, _encoding, _connectionId, 
                                           _compress));
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
        if(!(endpoint instanceof TcpEndpointI))
        {
            return false;
        }
        TcpEndpointI tcpEndpointI = (TcpEndpointI)endpoint;
        return tcpEndpointI._host.equals(_host) && tcpEndpointI._port == _port;
    }

    public int
    hashCode()
    {
        return _hashCode;
    }

    //
    // Compare endpoints for sorting purposes
    //
    public int
    compareTo(EndpointI obj) // From java.lang.Comparable
    {
        if(!(obj instanceof TcpEndpointI))
        {
            return type() < obj.type() ? -1 : 1;
        }

        TcpEndpointI p = (TcpEndpointI)obj;
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

        if(_timeout < p._timeout)
        {
            return -1;
        }
        else if(p._timeout < _timeout)
        {
            return 1;
        }

        if(!_connectionId.equals(p._connectionId))
        {
            return _connectionId.compareTo(p._connectionId);
        }

        if(!_compress && p._compress)
        {
            return -1;
        }
        else if(!p._compress && _compress)
        {
            return 1;
        }

        return _host.compareTo(p._host);
    }

    public java.util.List<Connector>
    connectors(java.util.List<java.net.InetSocketAddress> addresses)
    {
        java.util.List<Connector> connectors = new java.util.ArrayList<Connector>();
        for(java.net.InetSocketAddress p : addresses)
        {
            connectors.add(new TcpConnector(_instance, p, _timeout, _protocol, _encoding, _connectionId));
        }
        return connectors;
    }

    private void
    calcHashValue()
    {
        _hashCode = _host.hashCode();
        _hashCode = 5 * _hashCode + _port;
        _hashCode = 5 * _hashCode + _timeout;
        _hashCode = 5 * _hashCode + _protocol.hashCode();
        _hashCode = 5 * _hashCode + _encoding.hashCode();
        _hashCode = 5 * _hashCode + _connectionId.hashCode();
        _hashCode = 5 * _hashCode + (_compress ? 1 : 0);
    }

    private Instance _instance;
    private String _host;
    private int _port;
    private int _timeout;
    private String _connectionId = "";
    private boolean _compress;
    private int _hashCode;
}
