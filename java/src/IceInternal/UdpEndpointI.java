// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class UdpEndpointI extends EndpointI
{
    final static short TYPE = 3;

    public
    UdpEndpointI(Instance instance, String ho, int po, String conId, boolean co, boolean pub)
    {
        _instance = instance;
        _host = ho;
        _port = po;
	_protocolMajor = Protocol.protocolMajor;
	_protocolMinor = Protocol.protocolMinor;
	_encodingMajor = Protocol.encodingMajor;
	_encodingMinor = Protocol.encodingMinor;
        _connect = false;
	_connectionId = conId;
	_compress = co;
	_publish = pub;
        calcHashValue();
    }

    public
    UdpEndpointI(Instance instance, String str, boolean adapterEndp)
    {
        _instance = instance;
        _host = null;
        _port = 0;
	_protocolMajor = Protocol.protocolMajor;
	_protocolMinor = Protocol.protocolMinor;
	_encodingMajor = Protocol.encodingMajor;
	_encodingMinor = Protocol.encodingMinor;
        _connect = false;
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
		e.str = "udp " + str;
		throw e;
            }

            String argument = null;
            if(i < arr.length && arr[i].charAt(0) != '-')
            {
                argument = arr[i++];
            }

            switch(option.charAt(1))
            {
                case 'v':
                {
                    if(argument == null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "udp " + str;
			throw e;
                    }

		    int pos = argument.indexOf('.');
                    if(pos == -1)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "udp " + str;
			throw e;
                    }

		    String majStr = argument.substring(0, pos);
		    String minStr = argument.substring(pos + 1, argument.length());
		    int majVersion;
		    int minVersion;
		    try
		    {
		        majVersion = Integer.parseInt(majStr);
			minVersion = Integer.parseInt(minStr);
		    }
                    catch(NumberFormatException ex)
		    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "udp " + str;
			throw e;
		    }

		    if(majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
		    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "udp " + str;
			throw e;
		    }

		    if(majVersion != Protocol.protocolMajor)
		    {
			Ice.UnsupportedProtocolException e = new Ice.UnsupportedProtocolException();
			e.badMajor = majVersion < 0 ? majVersion + 255 : majVersion;
			e.badMinor = minVersion < 0 ? minVersion + 255 : minVersion;
			e.major = Protocol.protocolMajor;
			e.minor = Protocol.protocolMinor;
			throw e;
		    }

		    _protocolMajor = (byte)majVersion;
		    _protocolMinor = (byte)minVersion;

                    break;
                }

                case 'e':
                {
                    if(argument == null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "udp " + str;
			throw e;
                    }

		    int pos = argument.indexOf('.');
                    if(pos == -1)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "udp " + str;
			throw e;
                    }

		    String majStr = argument.substring(0, pos);
		    String minStr = argument.substring(pos + 1, argument.length());
		    int majVersion;
		    int minVersion;
		    try
		    {
		        majVersion = Integer.parseInt(majStr);
			minVersion = Integer.parseInt(minStr);
		    }
                    catch(NumberFormatException ex)
		    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "udp " + str;
			throw e;
		    }

		    if(majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
		    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "udp " + str;
			throw e;
		    }

		    if(majVersion != Protocol.encodingMajor)
		    {
			Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
			e.badMajor = majVersion < 0 ? majVersion + 255 : majVersion;
			e.badMinor = minVersion < 0 ? minVersion + 255 : minVersion;
			e.major = Protocol.encodingMajor;
			e.minor = Protocol.encodingMinor;
			throw e;
		    }

		    _encodingMajor = (byte)majVersion;
		    _encodingMinor = (byte)minVersion;

                    break;
                }

                case 'h':
                {
                    if(argument == null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "udp " + str;
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
			e.str = "udp " + str;
			throw e;
                    }

                    try
                    {
                        _port = Integer.parseInt(argument);
                    }
                    catch(NumberFormatException ex)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "udp " + str;
			throw e;
                    }

                    break;
                }

                case 'c':
                {
                    if(argument != null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "udp " + str;
			throw e;
                    }

                    _connect = true;
                    break;
                }

                case 'z':
                {
                    if(argument != null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "udp " + str;
			throw e;
                    }

                    _compress = true;
                    break;
                }

                default:
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
		    e.str = "udp " + str;
		    throw e;
                }
            }
        }

        if(_host == null)
        {
            _host = instance.defaultsAndOverrides().defaultHost;
            if(_host == null)
            {
	        if(adapterEndp)
		{
		    _host = "0.0.0.0";
		}
		else
		{
                    _host = Network.getLocalHost(true);
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
    UdpEndpointI(BasicStream s)
    {
        _instance = s.instance();
        s.startReadEncaps();
        _host = s.readString();
        _port = s.readInt();
	_protocolMajor = s.readByte();
	_protocolMinor = s.readByte();
	_encodingMajor = s.readByte();
	_encodingMinor = s.readByte();
	if(_protocolMajor != Protocol.protocolMajor)
	{
	    Ice.UnsupportedProtocolException e = new Ice.UnsupportedProtocolException();
	    e.badMajor = _protocolMajor < 0 ? _protocolMajor + 255 : _protocolMajor;
	    e.badMinor = _protocolMinor < 0 ? _protocolMinor + 255 : _protocolMinor;
	    e.major = Protocol.protocolMajor;
	    e.minor = Protocol.protocolMinor;
	    throw e;
	}
	if(_encodingMajor != Protocol.encodingMajor)
	{
	    Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
	    e.badMajor = _encodingMajor < 0 ? _encodingMajor + 255 : _encodingMajor;
	    e.badMinor = _encodingMinor < 0 ? _encodingMinor + 255 : _encodingMinor;
	    e.major = Protocol.encodingMajor;
	    e.minor = Protocol.encodingMinor;
	    throw e;
	}
        // Not transmitted.
        //_connect = s.readBool();
        _connect = false;
	_compress = s.readBool();
        s.endReadEncaps();
	_publish = true;
        calcHashValue();
    }

    //
    // Marshal the endpoint
    //
    public void
    streamWrite(BasicStream s)
    {
        s.writeShort(TYPE);
        s.startWriteEncaps();
        s.writeString(_host);
        s.writeInt(_port);
	s.writeByte(_protocolMajor);
	s.writeByte(_protocolMinor);
	s.writeByte(_encodingMajor);
	s.writeByte(_encodingMinor);
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
        String s = "udp";

	if((int)_protocolMajor != 1 || (int)_protocolMinor != 0)
	{
	    s += " -v " + (_protocolMajor < 0 ? (int)_protocolMajor + 255 : _protocolMajor)
		+ "." + (_protocolMinor < 0 ? (int)_protocolMinor + 255 : _protocolMinor);
	}

	if((int)_encodingMajor != 1 || (int)_encodingMinor != 0)
	{
	    s += " -e " + (_encodingMajor < 0 ? (int)_encodingMajor + 255 : _encodingMajor)
		+ "." + (_encodingMinor < 0 ? (int)_encodingMinor + 255 : _encodingMinor);
	}

	s += " -h " + _host + " -p " + _port;

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
            return new UdpEndpointI(_instance, _host, _port, _connectionId, compress, _publish);
        }
    }

    //
    // Return a new endpoint with a different connection id.
    //
    public EndpointI
    connectionId(String connectionId)
    {
        if(connectionId == _connectionId)
        {
            return this;
        }
        else
        {
            return new UdpEndpointI(_instance, _host, _port, connectionId, _compress, _publish);
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
    public Transceiver
    clientTransceiver()
    {
        return new UdpTransceiver(_instance, _host, _port);
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public Transceiver
    serverTransceiver(EndpointIHolder endpoint)
    {
        UdpTransceiver p = new UdpTransceiver(_instance, _host, _port, _connect);
        endpoint.value = new UdpEndpointI(_instance, _host, p.effectivePort(), _connectionId, _compress, _publish);
        return p;
    }

    //
    // Return a connector for this endpoint, or null if no connector
    // is available.
    //
    public Connector
    connector()
    {
        return null;
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    public Acceptor
    acceptor(EndpointIHolder endpoint)
    {
        endpoint.value = this;
        return null;
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
            java.util.ArrayList hosts = Network.getLocalHosts();
            java.util.Iterator iter = hosts.iterator();
            while(iter.hasNext())
            {
                String host = (String)iter.next();
                endps.add(new UdpEndpointI(_instance, host, _port, _connectionId, _compress,
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
    equivalent(Transceiver transceiver)
    {
        UdpTransceiver udpTransceiver = null;
        try
        {
            udpTransceiver = (UdpTransceiver)transceiver;
        }
        catch(ClassCastException ex)
        {
            return false;
        }
        return udpTransceiver.equivalent(_host, _port);
    }

    public boolean
    equivalent(Acceptor acceptor)
    {
        return false;
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
        UdpEndpointI p = null;

        try
        {
            p = (UdpEndpointI)obj;
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

        if(!_connect && p._connect)
        {
            return -1;
        }
        else if(!p._connect && _connect)
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

        if(!_host.equals(p._host))
        {
            //
            // We do the most time-consuming part of the comparison last.
            //
            java.net.InetSocketAddress laddr = null;
	    try
	    {
		laddr = Network.getAddress(_host, _port);
	    }
	    catch(Ice.DNSException ex)
	    {
	    }

            java.net.InetSocketAddress raddr = null;
	    try
	    {
		raddr = Network.getAddress(p._host, p._port);
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
        _hashCode = _host.hashCode();
        _hashCode = 5 * _hashCode + _port;
        _hashCode = 5 * _hashCode + (_connect ? 1 : 0);
        _hashCode = 5 * _hashCode + _connectionId.hashCode();
        _hashCode = 5 * _hashCode + (_compress ? 1 : 0);
    }

    private Instance _instance;
    private String _host;
    private int _port;
    private byte _protocolMajor;
    private byte _protocolMinor;
    private byte _encodingMajor;
    private byte _encodingMinor;
    private boolean _connect;
    private String _connectionId = "";
    private boolean _compress;
    private boolean _publish;
    private int _hashCode;
}
