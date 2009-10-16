// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class EndpointI extends IceInternal.EndpointI
{
    final static short TYPE = 2;

    public
    EndpointI(Instance instance, String ho, int po, int ti, String conId, boolean co)
    {
        _instance = instance;
        _host = ho;
        _port = po;
        _timeout = ti;
        _connectionId = conId;
        _compress = co;
        calcHashValue();
    }

    public
    EndpointI(Instance instance, String str, boolean oaEndpoint)
    {
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
                throw new Ice.EndpointParseException("ssl " + str);
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
                        throw new Ice.EndpointParseException("ssl " + str);
                    }

                    _host = argument;
                    break;
                }

                case 'p':
                {
                    if(argument == null)
                    {
                        throw new Ice.EndpointParseException("ssl " + str);
                    }

                    try
                    {
                        _port = Integer.parseInt(argument);
                    }
                    catch(NumberFormatException ex)
                    {
                        throw new Ice.EndpointParseException("ssl " + str);
                    }

                    if(_port < 0 || _port > 65535)
                    {
                        throw new Ice.EndpointParseException("ssl " + str);
                    }

                    break;
                }

                case 't':
                {
                    if(argument == null)
                    {
                        throw new Ice.EndpointParseException("ssl " + str);
                    }

                    try
                    {
                        _timeout = Integer.parseInt(argument);
                    }
                    catch(NumberFormatException ex)
                    {
                        throw new Ice.EndpointParseException("ssl " + str);
                    }

                    break;
                }

                case 'z':
                {
                    if(argument != null)
                    {
                        throw new Ice.EndpointParseException("ssl " + str);
                    }

                    _compress = true;
                    break;
                }

                default:
                {
                    throw new Ice.EndpointParseException("ssl " + str);
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
                throw new Ice.EndpointParseException("ssl " + str);
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
        s.writeShort(TYPE);
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
        return new IceSSL.SSLEndpointInfo(_timeout, _compress, _host, _port)
            {
                public short type()
                {
                    return TYPE;
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
        return TYPE;
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
    connectors()
    {
        return connectors(IceInternal.Network.getAddresses(_host, _port, _instance.protocolSupport()));
    }

    public void
    connectors_async(IceInternal.EndpointI_connectors callback)
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
    public IceInternal.Acceptor
    acceptor(IceInternal.EndpointIHolder endpoint, String adapterName)
    {
        AcceptorI p = new AcceptorI(_instance, adapterName, _host, _port);
        endpoint.value = 
            new EndpointI(_instance, _host, p.effectivePort(), _timeout, _connectionId, _compress);
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
        EndpointI sslEndpointI = null;
        try
        {
            sslEndpointI = (EndpointI)endpoint;
        }
        catch(ClassCastException ex)
        {
            return false;
        }
        return sslEndpointI._host.equals(_host) && sslEndpointI._port == _port;
    }

    public int
    hashCode()
    {
        return _hashCode;
    }

    //
    // Compare endpoints for sorting purposes
    //
    public boolean
    equals(java.lang.Object obj)
    {
        try
        {
            return compareTo((IceInternal.EndpointI)obj) == 0;
        }
        catch(ClassCastException ee)
        {
            assert(false);
            return false;
        }
    }

    public int
    compareTo(IceInternal.EndpointI obj) // From java.lang.Comparable
    {
        EndpointI p = null;

        try
        {
            p = (EndpointI)obj;
        }
        catch(ClassCastException ex)
        {
            return type() < obj.type() ? -1 : 1;
        }

        if(this == p)
        {
            return 0;
        }

        if(_port < p._port)
        {
            return -1;
        }
        else if(p._port < _port)
        {
            return 1;
        }

        if(!_connectionId.equals(p._connectionId))
        {
            return _connectionId.compareTo(p._connectionId);
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

    public java.util.List<IceInternal.Connector>
    connectors(java.util.List<java.net.InetSocketAddress> addresses)
    {
        java.util.List<IceInternal.Connector> connectors = new java.util.ArrayList<IceInternal.Connector>();
        for(java.net.InetSocketAddress p : addresses)
        {
            connectors.add(new ConnectorI(_instance, _host, p, _timeout, _connectionId));
        }
        return connectors;
    }

    private void
    calcHashValue()
    {
        _hashCode = _host.hashCode();
        _hashCode = 5 * _hashCode + _port;
        _hashCode = 5 * _hashCode + _timeout;
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
