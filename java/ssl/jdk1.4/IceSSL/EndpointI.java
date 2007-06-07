// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    EndpointI(Instance instance, String ho, int po, int ti, String conId, boolean co, boolean oae)
    {
        _instance = instance;
        _host = ho;
        _port = po;
        _timeout = ti;
        _connectionId = conId;
        _compress = co;
        _oaEndpoint = oae;
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
        _oaEndpoint = oaEndpoint;

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
            if(_host == null)
            {
                if(_oaEndpoint)
                {
                    _host = "0.0.0.0";
                }
                else
                {
                    _host = "127.0.0.1";
                }
            }
        }
        else if(_host.equals("*"))
        {
            _host = "0.0.0.0";
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
        _oaEndpoint = false;
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
        String s = "ssl -h " + _host + " -p " + _port;
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
            return new EndpointI(_instance, _host, _port, timeout, _connectionId, _compress, _oaEndpoint);
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
            return new EndpointI(_instance, _host, _port, _timeout, connectionId, _compress, _oaEndpoint);
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
            return new EndpointI(_instance, _host, _port, _timeout, _connectionId, compress, _oaEndpoint);
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
    // Return true if the endpoint type is unknown.
    //
    public boolean
    unknown()
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
    public java.util.ArrayList
    connectors()
    {
        java.util.ArrayList connectors = new java.util.ArrayList();
        java.util.ArrayList addresses = IceInternal.Network.getAddresses(_host, _port);
        java.util.Iterator p = addresses.iterator();
        while(p.hasNext())
        {
            connectors.add(new ConnectorI(_instance, (java.net.InetSocketAddress)p.next(), _timeout, _connectionId));
        }
        return connectors;
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
        endpoint.value = new EndpointI(_instance, _host, p.effectivePort(), _timeout, _connectionId, _compress,
                                       _oaEndpoint);
        return p;
    }

    //
    // Expand endpoint out in to separate endpoints for each local
    // host if endpoint was configured with no host set.
    //
    public java.util.ArrayList
    expand()
    {
        java.util.ArrayList endps = new java.util.ArrayList();
        if(_host.equals("0.0.0.0"))
        {
            java.util.ArrayList hosts = IceInternal.Network.getLocalHosts();
            java.util.Iterator iter = hosts.iterator();
            while(iter.hasNext())
            {
                String host = (String)iter.next();
                if(!_oaEndpoint || hosts.size() == 1 || !host.equals("127.0.0.1"))
                {
                    endps.add(new EndpointI(_instance, host, _port, _timeout, _connectionId, _compress, _oaEndpoint));
                                        
                }
            }
        }
        else
        {
            endps.add(this);
        }
        return endps;
    }

    //
    // Check whether the endpoint is equivalent to a specific
    // Transceiver or Acceptor
    //
    public boolean
    equivalent(IceInternal.Transceiver transceiver)
    {
        return false;
    }

    public boolean
    equivalent(IceInternal.Acceptor acceptor)
    {
        AcceptorI sslAcceptor = null;
        try
        {
            sslAcceptor = (AcceptorI)acceptor;
        }
        catch(ClassCastException ex)
        {
            return false;
        }
        return sslAcceptor.equivalent(_host, _port);
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
        return compareTo(obj) == 0;
    }

    public int
    compareTo(java.lang.Object obj) // From java.lang.Comparable
    {
        EndpointI p = null;

        try
        {
            p = (EndpointI)obj;
        }
        catch(ClassCastException ex)
        {
            try
            {
                IceInternal.EndpointI e = (IceInternal.EndpointI)obj;
                return type() < e.type() ? -1 : 1;
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

    public boolean
    requiresThreadPerConnection()
    {
        return true;
    }

    private void
    calcHashValue()
    {
        try
        {
            java.net.InetSocketAddress addr = IceInternal.Network.getAddress(_host, _port);
            _hashCode = addr.getAddress().getHostAddress().hashCode();
        }
        catch(Ice.DNSException ex)
        {
            _hashCode = _host.hashCode();
        }
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
    private boolean _oaEndpoint;
    private int _hashCode;
}
