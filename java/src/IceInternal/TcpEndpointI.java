// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class TcpEndpointI extends EndpointI
{
    public
    TcpEndpointI(Instance instance, String ho, int po, int ti, String conId, boolean co)
    {
        super(conId);
        _instance = instance;
        _host = ho;
        _port = po;
        _timeout = ti;
        _compress = co;
        calcHashValue();
    }

    public
    TcpEndpointI(Instance instance, String str, boolean oaEndpoint)
    {
        super("");
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
                    throw new Ice.EndpointParseException("unknown option `" + option + "' in `tcp " + str + "'");
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
        super("");
        _instance = s.instance();
        s.startReadEncaps();
        _host = s.readString();
        _port = s.readInt();
        _timeout = s.readInt();
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
        s.writeShort(Ice.TCPEndpointType.value);
        s.startWriteEncaps();
        s.writeString(_host);
        s.writeInt(_port);
        s.writeInt(_timeout);
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
        String s = "tcp";

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
        return new Ice.TCPEndpointInfo(_timeout, _compress, _host, _port)
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
    // Return the protocol name
    //
    public String
    protocol()
    {
        return "tcp";
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
            return new TcpEndpointI(_instance, _host, _port, timeout, _connectionId, _compress);
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
            return new TcpEndpointI(_instance, _host, _port, _timeout, connectionId, _compress);
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
            return new TcpEndpointI(_instance, _host, _port, _timeout, _connectionId, compress);
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
    connectors(Ice.EndpointSelectionType selType)
    {
        return _instance.endpointHostResolver().resolve(_host, _port, selType, this);
    }

    public void
    connectors_async(Ice.EndpointSelectionType selType, EndpointI_connectors callback)
    {
        _instance.endpointHostResolver().resolve(_host, _port, selType, this, callback);
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
        endpoint.value = new TcpEndpointI(_instance, _host, p.effectivePort(), _timeout, _connectionId, _compress);
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
                endps.add(new TcpEndpointI(_instance, h, _port, _timeout, _connectionId, _compress));
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

    public java.util.List<Connector>
    connectors(java.util.List<java.net.InetSocketAddress> addresses, NetworkProxy proxy)
    {
        java.util.List<Connector> connectors = new java.util.ArrayList<Connector>();
        for(java.net.InetSocketAddress p : addresses)
        {
            connectors.add(new TcpConnector(_instance, p, proxy, _timeout, _connectionId));
        }
        return connectors;
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

    private void
    calcHashValue()
    {
        int h = 5381;
        h = IceInternal.HashUtil.hashAdd(h, Ice.TCPEndpointType.value);
        h = IceInternal.HashUtil.hashAdd(h, _host);
        h = IceInternal.HashUtil.hashAdd(h, _port);
        h = IceInternal.HashUtil.hashAdd(h, _timeout);
        h = IceInternal.HashUtil.hashAdd(h, _connectionId);
        h = IceInternal.HashUtil.hashAdd(h, _compress);
        _hashCode = h;
    }

    private Instance _instance;
    private String _host;
    private int _port;
    private int _timeout;
    private boolean _compress;
    private int _hashCode;
}
