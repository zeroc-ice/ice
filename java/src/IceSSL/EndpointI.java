// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class EndpointI extends IceInternal.EndpointI
{
    public
    EndpointI(Instance instance, String ho, int po, int ti, String conId, boolean co)
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
    EndpointI(Instance instance, String str, boolean oaEndpoint)
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
                                                     "' in endpoint `ssl " + str + "'");
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
                        throw new Ice.EndpointParseException("no argument provided for -h option in endpoint `ssl "
                                                             + str + "'");
                    }

                    _host = argument;
                    break;
                }

                case 'p':
                {
                    if(argument == null)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -p option in endpoint `ssl "
                                                             + str + "'");
                    }

                    try
                    {
                        _port = Integer.parseInt(argument);
                    }
                    catch(NumberFormatException ex)
                    {
                        throw new Ice.EndpointParseException("invalid port value `" + argument +
                                                             "' in endpoint `ssl " + str + "'");
                    }

                    if(_port < 0 || _port > 65535)
                    {
                        throw new Ice.EndpointParseException("port value `" + argument +
                                                             "' out of range in endpoint `ssl " + str + "'");
                    }

                    break;
                }

                case 't':
                {
                    if(argument == null)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -t option in endpoint `ssl "
                                                             + str + "'");
                    }

                    try
                    {
                        _timeout = Integer.parseInt(argument);
                    }
                    catch(NumberFormatException ex)
                    {
                        throw new Ice.EndpointParseException("invalid timeout value `" + argument +
                                                             "' in endpoint `ssl " + str + "'");
                    }

                    break;
                }

                case 'z':
                {
                    if(argument != null)
                    {
                        throw new Ice.EndpointParseException("unexpected argument `" + argument +
                                                             "' provided for -z option in `ssl " + str + "'");
                    }

                    _compress = true;
                    break;
                }

                default:
                {
                    throw new Ice.EndpointParseException("unknown option `" + option + "' in `ssl " + str + "'");
                }
            }
        }

        if(_host == null)
        {
            _host = _instance.defaultHost();
        }
        else if(_host.equals("*"))
        {
            if(oaEndpoint)
            {
                _host = null;
            }
            else
            {
                throw new Ice.EndpointParseException("`-h *' not valid for proxy endpoint `ssl " + str + "'");
            }
        }

        if(_host == null)
        {
            _host = "";
        }

        calcHashValue();
    }

    public
    EndpointI(Instance instance, IceInternal.BasicStream s)
    {
        super("");
        _instance = instance;
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
    streamWrite(IceInternal.BasicStream s)
    {
        s.writeShort(EndpointType.value);
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
        String s = "ssl";

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
        return new IceSSL.EndpointInfo(_timeout, _compress, _host, _port)
            {
                public short type()
                {
                    return EndpointType.value;
                }
                
                public boolean datagram()
                {
                    return false;
                }
                
                public boolean secure()
                {
                    return true;
                }
        };
    }

    //
    // Return the endpoint type
    //
    public short
    type()
    {
        return EndpointType.value;
    }

    //
    // Return the protocol name
    //
    public String
    protocol()
    {
        return "ssl";
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
    public IceInternal.EndpointI
    timeout(int timeout)
    {
        if(timeout == _timeout)
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _host, _port, timeout, _connectionId, _compress);
        }
    }

    //
    // Return a new endpoint with a different connection id.
    //
    public IceInternal.EndpointI
    connectionId(String connectionId)
    {
        if(connectionId.equals(_connectionId))
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _host, _port, _timeout, connectionId, _compress);
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
    public IceInternal.EndpointI
    compress(boolean compress)
    {
        if(compress == _compress)
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _host, _port, _timeout, _connectionId, compress);
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
        return true;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public IceInternal.Transceiver
    transceiver(IceInternal.EndpointIHolder endpoint)
    {
        endpoint.value = this;
        return null;
    }

    //
    // Return connectors for this endpoint, or empty list if no connector
    // is available.
    //
    public java.util.List<IceInternal.Connector>
    connectors(Ice.EndpointSelectionType selType)
    {
        return _instance.endpointHostResolver().resolve(_host, _port, selType, this);
    }

    public void
    connectors_async(Ice.EndpointSelectionType selType, IceInternal.EndpointI_connectors callback)
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
    public IceInternal.Acceptor
    acceptor(IceInternal.EndpointIHolder endpoint, String adapterName)
    {
        AcceptorI p = new AcceptorI(_instance, adapterName, _host, _port);
        endpoint.value = new EndpointI(_instance, _host, p.effectivePort(), _timeout, _connectionId, _compress);
        return p;
    }

    //
    // Expand endpoint out in to separate endpoints for each local
    // host if listening on INADDR_ANY.
    //
    public java.util.List<IceInternal.EndpointI>
    expand()
    {
        java.util.ArrayList<IceInternal.EndpointI> endps = new java.util.ArrayList<IceInternal.EndpointI>();
        java.util.ArrayList<String> hosts =
            IceInternal.Network.getHostsForEndpointExpand(_host, _instance.protocolSupport(), false);
        if(hosts == null || hosts.isEmpty())
        {
            endps.add(this);
        }
        else
        {
            for(String host : hosts)
            {
                endps.add(new EndpointI(_instance, host, _port, _timeout, _connectionId, _compress));
            }
        }
        return endps;
    }

    //
    // Check whether the endpoint is equivalent to a specific Connector.
    //
    public boolean
    equivalent(IceInternal.EndpointI endpoint)
    {
        if(!(endpoint instanceof EndpointI))
        {
            return false;
        }
        EndpointI sslEndpointI = (EndpointI)endpoint;
        return sslEndpointI._host.equals(_host) && sslEndpointI._port == _port;
    }

    public java.util.List<IceInternal.Connector>
    connectors(java.util.List<java.net.InetSocketAddress> addresses, IceInternal.NetworkProxy proxy)
    {
        java.util.List<IceInternal.Connector> connectors = new java.util.ArrayList<IceInternal.Connector>();
        for(java.net.InetSocketAddress p : addresses)
        {
            connectors.add(new ConnectorI(_instance, _host, p, proxy, _timeout, _connectionId));
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
    compareTo(IceInternal.EndpointI obj) // From java.lang.Comparable
    {
        if(!(obj instanceof EndpointI))
        {
            return type() < obj.type() ? -1 : 1;
        }

        EndpointI p = (EndpointI)obj;
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
        h = IceInternal.HashUtil.hashAdd(h, EndpointType.value);
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
