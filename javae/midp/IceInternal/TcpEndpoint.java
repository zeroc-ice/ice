// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// MIDP Version of TcpEndpoint
//

package IceInternal;

final class TcpEndpoint implements Endpoint
{
    final static short TYPE = 1;

    public
    TcpEndpoint(Instance instance, String ho, int po, int ti)
    {
        _instance = instance;
        _host = ho;
        _port = po;
        _timeout = ti;
        calcHashValue();
    }

    public
    TcpEndpoint(Instance instance, String str)
    {
        _instance = instance;
        _host = null;
        _port = 0;
        _timeout = -1;

        String[] arr = IceUtil.StringUtil.split(str, " \t\n\r");

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
		e.str = "tcp " + str;
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
			e.str = "tcp " + str;
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
			e.str = "tcp " + str;
			throw e;
                    }

                    try
                    {
                        _port = Integer.parseInt(argument);
                    }
                    catch(NumberFormatException ex)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "tcp " + str;
			throw e;
                    }

		    if(_port < 0 || _port > 65535)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "tcp " + str;
			throw e;
                    }

                    break;
                }

                case 't':
                {
                    if(argument == null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "tcp " + str;
			throw e;
                    }

                    try
                    {
                        _timeout = Integer.parseInt(argument);
                    }
                    catch(NumberFormatException ex)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
			e.str = "tcp " + str;
			throw e;
                    }

                    break;
                }

		case 'z':
		{
		    // Ignore compression flag.
		    break;
		}

                default:
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
		    e.str = "tcp " + str;
		    throw e;
                }
            }
        }

        if(_host == null)
        {
            _host = _instance.defaultsAndOverrides().defaultHost;
        }

        calcHashValue();
    }

    public
    TcpEndpoint(BasicStream s)
    {
        _instance = s.instance();
        s.startReadEncaps();
        _host = s.readString();
        _port = s.readInt();
        _timeout = s.readInt();
	boolean compress = s.readBool();
        s.endReadEncaps();
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
        s.writeInt(_timeout);
	s.writeBool(false);
        s.endWriteEncaps();
    }

    //
    // Convert the endpoint to its string form
    //
    public String
    toString()
    {
        String s = "tcp -h " + _host + " -p " + _port;
        if(_timeout != -1)
        {
            s += " -t " + _timeout;
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
    public Endpoint
    timeout(int timeout)
    {
        if(timeout == _timeout)
        {
            return this;
        }
        else
        {
            return new TcpEndpoint(_instance, _host, _port, timeout);
        }
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
    // Return true if the endpoint is datagram-based.
    //
    public boolean
    datagram()
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
        return null;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public Transceiver
    serverTransceiver(EndpointHolder endpoint)
    {
        endpoint.value = this;
        return null;
    }

    //
    // Return a connector for this endpoint, or null if no connector
    // is available.
    //
    public Connector
    connector()
    {
        return new Connector(_instance, _host, _port);
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    public Acceptor
    acceptor(EndpointHolder endpoint)
    {
        Acceptor p = new Acceptor(_instance, _host, _port);
        endpoint.value = new TcpEndpoint(_instance, _host, p.effectivePort(), _timeout);
        return p;
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
        TcpEndpoint p = null;

        try
        {
            p = (TcpEndpoint)obj;
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

        if(_timeout < p._timeout)
        {
            return -1;
        }
        else if(p._timeout < _timeout)
        {
            return 1;
        }

	byte[] myIP = IPAddr();
	if(myIP != null)
	{
	    byte[] otherIP = p.IPAddr();
	    if(otherIP != null)
	    {
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(myIP.length == otherIP.length);
		}
		for(int i = 0; i < otherIP.length; i++)
		{
		    if(myIP[i] < otherIP[i])
		    {
			return -1;
		    }
		    else if(otherIP[i] < myIP[i])
		    {
			return 1;
		    }
		}
	    }
	}

	//
	// At this point the best we can do is a lexical compare.
	//
        return _host.compareTo(p._host);
    }

    private byte[]
    IPAddr()
    {
	if(_ip == null && !_parsed)
	{
	    _parsed = true;
	    _ip = Network.addrStringToIP(_host);
	}
	return _ip;
    }
	
    private void
    calcHashValue()
    {
        _hashCode = _host.hashCode();
        _hashCode = 5 * _hashCode + _port;
        _hashCode = 5 * _hashCode + _timeout;
    }

    private Instance _instance;
    private String _host;
    private int _port;
    private int _timeout;
    private int _hashCode;
    private byte[] _ip;
    private boolean _parsed = false;
}
