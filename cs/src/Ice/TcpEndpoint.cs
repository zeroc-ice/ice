// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Diagnostics;

    sealed class TcpEndpoint : Endpoint
    {
	internal const short TYPE = 1;
	
	public TcpEndpoint(Instance instance, string ho, int po, int ti, bool co)
	{
	    _instance = instance;
	    _host = ho;
	    _port = po;
	    _timeout = ti;
	    _compress = co;
	    calcHashValue();
	}
	
	public TcpEndpoint(Instance instance, string str)
	{
	    _instance = instance;
	    _host = null;
	    _port = 0;
	    _timeout = -1;
	    _compress = false;
	    
	    char[] separators = { ' ', '\t', '\n', '\r' };
	    string[] arr = str.Split(separators);
	    
	    int i = 0;
	    while(i < arr.Length)
	    {
		if(arr[i].Length == 0)
		{
		    i++;
		    continue;
		}
		
		string option = arr[i++];
		if(option.Length != 2 || option[0] != '-')
		{
		    Ice.EndpointParseException e = new Ice.EndpointParseException();
		    e.str = "tcp " + str;
		    throw e;
		}
		
		string argument = null;
		if(i < arr.Length && arr[i][0] != '-')
		{
		    argument = arr[i++];
		}
		
		switch(option[1])
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
			    _port = System.Int32.Parse(argument);
			}
			catch(System.FormatException ex)
			{
			    Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
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
			    _timeout = System.Int32.Parse(argument);
			}
			catch(System.FormatException ex)
			{
			    Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
			    e.str = "tcp " + str;
			    throw e;
			}
			
			break;
		    }
		    
		    case 'z': 
		    {
			if(argument != null)
			{
			    Ice.EndpointParseException e = new Ice.EndpointParseException();
			    e.str = "tcp " + str;
			    throw e;
			}
			
			_compress = true;
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
	
	public TcpEndpoint(BasicStream s)
	{
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
	public void streamWrite(BasicStream s)
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
	public override string ToString()
	{
	    string s = "tcp -h " + _host + " -p " + _port;
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
	public short type()
	{
	    return TYPE;
	}
	
	//
	// Return the timeout for the endpoint in milliseconds. 0 means
	// non-blocking, -1 means no timeout.
	//
	public int timeout()
	{
	    return _timeout;
	}
	
	//
	// Return a new endpoint with a different timeout value, provided
	// that timeouts are supported by the endpoint. Otherwise the same
	// endpoint is returned.
	//
	public Endpoint timeout(int timeout)
	{
	    if(timeout == _timeout)
	    {
		return this;
	    }
	    else
	    {
		return new TcpEndpoint(_instance, _host, _port, timeout, _compress);
	    }
	}
	
	//
	// Return true if the endpoints support bzip2 compress, or false
	// otherwise.
	//
	public bool compress()
	{
	    return _compress;
	}
	
	//
	// Return a new endpoint with a different compression value,
	// provided that compression is supported by the
	// endpoint. Otherwise the same endpoint is returned.
	//
	public Endpoint compress(bool compress)
	{
	    if(compress == _compress)
	    {
		return this;
	    }
	    else
	    {
		return new TcpEndpoint(_instance, _host, _port, _timeout, compress);
	    }
	}
	
	//
	// Return true if the endpoint is datagram-based.
	//
	public bool datagram()
	{
	    return false;
	}
	
	//
	// Return true if the endpoint is secure.
	//
	public bool secure()
	{
	    return false;
	}
	
	//
	// Return true if the endpoint type is unknown.
	//
	public bool unknown()
	{
	    return false;
	}
	
	//
	// Return a client side transceiver for this endpoint, or null if a
	// transceiver can only be created by a connector.
	//
	public Transceiver clientTransceiver()
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
	public Transceiver serverTransceiver(ref Endpoint endpoint)
	{
	    endpoint = this;
	    return null;
	}
	
	//
	// Return a connector for this endpoint, or null if no connector
	// is available.
	//
	public Connector connector()
	{
	    return new TcpConnector(_instance, _host, _port);
	}
	
	//
	// Return an acceptor for this endpoint, or null if no acceptors
	// is available. In case an acceptor is created, this operation
	// also returns a new "effective" endpoint, which might differ
	// from this endpoint, for example, if a dynamic port number is
	// assigned.
	//
	public Acceptor acceptor(ref Endpoint endpoint)
	{
	    TcpAcceptor p = new TcpAcceptor(_instance, _host, _port);
	    endpoint = new TcpEndpoint(_instance, _host, p.effectivePort(), _timeout, _compress);
	    return p;
	}
	
	//
	// Check whether the endpoint is equivalent to a specific
	// Transceiver or Acceptor
	//
	public bool equivalent(Transceiver transceiver)
	{
	    return false;
	}
	
	public bool equivalent(Acceptor acceptor)
	{
	    TcpAcceptor tcpAcceptor = null;
	    try
	    {
		tcpAcceptor = (TcpAcceptor)acceptor;
	    }
	    catch(System.InvalidCastException)
	    {
		return false;
	    }
	    return tcpAcceptor.equivalent(_host, _port);
	}
	
	public override int GetHashCode()
	{
	    return _hashCode;
	}
	
	//
	// Compare endpoints for sorting purposes
	//
	public override bool Equals(object obj)
	{
	    return CompareTo(obj) == 0;
	}
	
	public int CompareTo(object obj)
	{
	    TcpEndpoint p = null;
	    
	    try
	    {
		p = (TcpEndpoint)obj;
	    }
	    catch(System.InvalidCastException)
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
	    
	    if(!_compress && p._compress)
	    {
		return -1;
	    }
	    else if(!p._compress && _compress)
	    {
		return 1;
	    }
	    
	    if(!_host.Equals(p._host))
	    {
		//
		// We do the most time-consuming part of the comparison last.
		//
		System.Net.IPEndPoint laddr = null;
		try
		{
		    laddr = Network.getAddress(_host, _port);
		}
		catch(Ice.DNSException)
		{
		}
		
		System.Net.IPEndPoint raddr = null;
		try
		{
		    raddr = Network.getAddress(p._host, p._port);
		}
		catch(Ice.DNSException)
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
		    byte[] larr = laddr.Address.GetAddressBytes();
		    byte[] rarr = raddr.Address.GetAddressBytes();
		    Debug.Assert(larr.Length == rarr.Length);
		    for(int i = 0; i < larr.Length; i++)
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
	
	private void calcHashValue()
	{
	    _hashCode = _host.GetHashCode();
	    _hashCode = 5 * _hashCode + _port;
	    _hashCode = 5 * _hashCode + _timeout;
	    _hashCode = 5 * _hashCode + (_compress? 1 : 0);
	}
	
	private Instance _instance;
	private string _host;
	private int _port;
	private int _timeout;
	private bool _compress;
	private int _hashCode;
    }

    sealed class TcpEndpointFactory : EndpointFactory
    {
        internal TcpEndpointFactory(Instance instance)
        {
            _instance = instance;
        }
	
        public short type()
        {
            return TcpEndpoint.TYPE;
        }
	
        public string protocol()
        {
            return "tcp";
        }
	
        public Endpoint create(string str)
        {
            return new TcpEndpoint(_instance, str);
        }
	
        public Endpoint read(BasicStream s)
        {
            return new TcpEndpoint(s);
        }
	
        public void destroy()
        {
            _instance = null;
        }
	
        private Instance _instance;
    }

}
