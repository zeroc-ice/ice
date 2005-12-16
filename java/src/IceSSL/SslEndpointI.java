// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class SslEndpointI extends IceInternal.EndpointI
{
    final static short TYPE = 2;

    public
    SslEndpointI(Instance instance, String ho, int po, int ti, String conId, boolean co, boolean pub)
    {
	_instance = instance;
	_host = ho;
	_port = po;
	_timeout = ti;
	_connectionId = conId;
	_compress = co;
	_publish = pub;
	calcHashValue();
    }

    public
    SslEndpointI(Instance instance, String str, boolean adapterEndp)
    {
	_instance = instance;
	_host = null;
	_port = 0;
	_timeout = -1;
	_compress = false;
	_publish = true;

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
		Ice.EndpointParseException e = new Ice.EndpointParseException();
		e.str = "ssl " + str;
		throw e;
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
			Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "ssl " + str;
			throw e;
		    }

		    _host = argument;
		    break;
		}

		case 'p':
		{
		    if(argument == null)
		    {
			Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "ssl " + str;
			throw e;
		    }

		    try
		    {
			_port = Integer.parseInt(argument);
		    }
		    catch(NumberFormatException ex)
		    {
			Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "ssl " + str;
			throw e;
		    }

		    break;
		}

		case 't':
		{
		    if(argument == null)
		    {
			Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "ssl " + str;
			throw e;
		    }

		    try
		    {
			_timeout = Integer.parseInt(argument);
		    }
		    catch(NumberFormatException ex)
		    {
			Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "ssl " + str;
			throw e;
		    }

		    break;
		}

		case 'z':
		{
		    if(argument != null)
		    {
			Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "ssl " + str;
			throw e;
		    }

		    _compress = true;
		    break;
		}

		default:
		{
		    Ice.EndpointParseException e = new Ice.EndpointParseException();
		    e.str = "ssl " + str;
		    throw e;
		}
	    }
	}

	if(_host == null)
	{
	    _host = _instance.defaultHost();
            if(_host == null)
            {
	        if(adapterEndp)
		{
		    _host = "0.0.0.0";
		}
		else
		{
                    _host = IceInternal.Network.getLocalHost(true);
		}
            }
	}
	else if(_host.equals("*") && adapterEndp)
	{
	    _host = "0.0.0.0";
	}

	calcHashValue();
    }

    public
    SslEndpointI(Instance instance, IceInternal.BasicStream s)
    {
	_instance = instance;
	s.startReadEncaps();
	_host = s.readString();
	_port = s.readInt();
	_timeout = s.readInt();
	_compress = s.readBool();
	s.endReadEncaps();
	_publish = true;
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
	    return new SslEndpointI(_instance, _host, _port, timeout, _connectionId, _compress, _publish);
	}
    }

    //
    // Return a new endpoint with a different connection id.
    //
    public IceInternal.EndpointI
    connectionId(String connectionId)
    {
	if(connectionId == _connectionId)
	{
	    return this;
	}
	else
	{
	    return new SslEndpointI(_instance, _host, _port, _timeout, connectionId, _compress, _publish);
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
	    return new SslEndpointI(_instance, _host, _port, _timeout, _connectionId, compress, _publish);
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
    // Return a client side transceiver for this endpoint, or null if a
    // transceiver can only be created by a connector.
    //
    public IceInternal.Transceiver
    clientTransceiver()
    {
	return null;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public IceInternal.Transceiver
    serverTransceiver(IceInternal.EndpointIHolder endpoint)
    {
	endpoint.value = this;
	return null;
    }

    //
    // Return a connector for this endpoint, or null if no connector
    // is available.
    //
    public IceInternal.Connector
    connector()
    {
	return new SslConnector(_instance, _host, _port);
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    public IceInternal.Acceptor
    acceptor(IceInternal.EndpointIHolder endpoint)
    {
	SslAcceptor p = new SslAcceptor(_instance, _host, _port);
	endpoint.value = new SslEndpointI(_instance, _host, p.effectivePort(), _timeout, _connectionId,
					  _compress, _publish);
	return p;
    }

    //
    // Expand endpoint out in to separate endpoints for each local
    // host if endpoint was configured with no host set. This
    // only applies for ObjectAdapter endpoints.
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
                endps.add(new SslEndpointI(_instance, host, _port, _timeout, _connectionId, _compress,
					   hosts.size() == 1 || !host.equals("127.0.0.1")));
            }
        }
        else
        {
            endps.add(this);
        }
        return endps;
    }

    //
    // Return whether endpoint should be published in proxies
    // created by Object Adapter.
    //
    public boolean
    publish()
    {
        return _publish;
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
	SslAcceptor sslAcceptor = null;
	try
	{
	    sslAcceptor = (SslAcceptor)acceptor;
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
	SslEndpointI p = null;

	try
	{
	    p = (SslEndpointI)obj;
	}
	catch(ClassCastException ex)
	{
	    return 1;
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

	if(!_host.equals(p._host))
	{
	    //
	    // We do the most time-consuming part of the comparison last.
	    //
	    java.net.InetSocketAddress laddr = null;
	    try
	    {
		laddr = IceInternal.Network.getAddress(_host, _port);
	    }
	    catch(Ice.DNSException ex)
	    {
	    }

	    java.net.InetSocketAddress raddr = null;
	    try
	    {
		raddr = IceInternal.Network.getAddress(p._host, p._port);
	    }
	    catch(Ice.DNSException ex)
	    {
	    }

	    if(laddr == null && raddr != null)
	    {
		return -1;
	    }
	    else if(raddr == null && laddr != null)
	    {
		return 1;
	    }
	    else if(laddr != null && raddr != null)
	    {
		byte[] larr = laddr.getAddress().getAddress();
		byte[] rarr = raddr.getAddress().getAddress();
		assert(larr.length == rarr.length);
		for(int i = 0; i < larr.length; i++)
		{
		    if(larr[i] < rarr[i])
		    {
			return -1;
		    }
		    else if(rarr[i] < larr[i])
		    {
			return 1;
		    }
		}
	    }
	}

	return 0;
    }

    private void
    calcHashValue()
    {
    	try
	{
            java.net.InetAddress addr = java.net.InetAddress.getByName(_host);
	    _hashCode = addr.getHostAddress().hashCode();
	}
	catch(java.net.UnknownHostException ex)
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
    private boolean _publish;
    private int _hashCode;
}
