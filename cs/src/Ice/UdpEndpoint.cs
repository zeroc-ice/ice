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

    sealed class UdpEndpoint : Endpoint
    {
	internal const short TYPE = 3;
	
	public UdpEndpoint(Instance instance, string ho, int po, bool co)
	{
	    _instance = instance;
	    _host = ho;
	    _port = po;
	    _protocolMajor = Protocol.protocolMajor;
	    _protocolMinor = Protocol.protocolMinor;
	    _encodingMajor = Protocol.encodingMajor;
	    _encodingMinor = Protocol.encodingMinor;
	    _connect = false;
	    _compress = co;
	    calcHashValue();
	}
	
	public UdpEndpoint(Instance instance, string str)
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
	    
	    char[] splitChars = { ' ', '\t', '\n', '\r' };
	    string[] arr = str.Split(splitChars);
	    
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
		    e.str = "udp " + str;
		    throw e;
		}
		
		string argument = null;
		if(i < arr.Length && arr[i][0] != '-')
		{
		    argument = arr[i++];
		}
		
		switch(option[1])
		{
		    case 'v': 
		    {
			if(argument == null)
			{
			    Ice.EndpointParseException e = new Ice.EndpointParseException();
			    e.str = "udp " + str;
			    throw e;
			}
			
			int pos = argument.IndexOf((System.Char) '.');
			if(pos == -1)
			{
			    Ice.EndpointParseException e = new Ice.EndpointParseException();
			    e.str = "udp " + str;
			    throw e;
			}
			
			string majStr = argument.Substring(0, (pos) - (0));
			string minStr = argument.Substring(pos + 1, (argument.Length) - (pos + 1));
			int majVersion;
			int minVersion;
			try
			{
			    majVersion = System.Int32.Parse(majStr);
			    minVersion = System.Int32.Parse(minStr);
			}
			catch(System.FormatException ex)
			{
			    Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
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
			    e.badMajor = majVersion < 0?majVersion + 255:majVersion;
			    e.badMinor = minVersion < 0?minVersion + 255:minVersion;
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
			
			int pos = argument.IndexOf((System.Char) '.');
			if(pos == -1)
			{
			    Ice.EndpointParseException e = new Ice.EndpointParseException();
			    e.str = "udp " + str;
			    throw e;
			}
			
			string majStr = argument.Substring(0, (pos) - (0));
			string minStr = argument.Substring(pos + 1, (argument.Length) - (pos + 1));
			int majVersion;
			int minVersion;
			try
			{
			    majVersion = System.Int32.Parse(majStr);
			    minVersion = System.Int32.Parse(minStr);
			}
			catch(System.FormatException ex)
			{
			    Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
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
			    e.badMajor = majVersion < 0?majVersion + 255:majVersion;
			    e.badMinor = minVersion < 0?minVersion + 255:minVersion;
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
			    _port = System.Int32.Parse(argument);
			}
			catch(System.FormatException ex)
			{
			    Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
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
	    }
	    
	    calcHashValue();
	}
	
	public UdpEndpoint(BasicStream s)
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
		e.badMajor = _protocolMajor < 0?_protocolMajor + 255:_protocolMajor;
		e.badMinor = _protocolMinor < 0?_protocolMinor + 255:_protocolMinor;
		e.major = Protocol.protocolMajor;
		e.minor = Protocol.protocolMinor;
		throw e;
	    }
	    if(_encodingMajor != Protocol.encodingMajor)
	    {
		Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
		e.badMajor = _encodingMajor < 0?_encodingMajor + 255:_encodingMajor;
		e.badMinor = _encodingMinor < 0?_encodingMinor + 255:_encodingMinor;
		e.major = Protocol.encodingMajor;
		e.minor = Protocol.encodingMinor;
		throw e;
	    }
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
	public void streamWrite(BasicStream s)
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
	public override string ToString()
	{
	    string s = "udp";

	    if((int)_protocolMajor != 1 || (int)_protocolMinor != 0)
	    {
	    	s += " -v " + (_protocolMajor < 0 ? (int)_protocolMajor + 255 : _protocolMajor);
	    	s += "." + (_protocolMinor < 0 ? (int)_protocolMinor + 255 : _protocolMinor);
	    }

	    if((int)_encodingMajor != 1 || (int)_encodingMinor != 0)
	    {
		s += " -e " + (_encodingMajor < 0 ? (int)_encodingMajor + 255 : _encodingMajor);
	    	s += "." + (_encodingMinor < 0 ? (int)_encodingMinor + 255 : _encodingMinor);
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
	    return -1;
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
		return new UdpEndpoint(_instance, _host, _port, compress);
	    }
	}
	
	//
	// Return a new endpoint with a different timeout value, provided
	// that timeouts are supported by the endpoint. Otherwise the same
	// endpoint is returned.
	//
	public Endpoint timeout(int timeout)
	{
	    return this;
	}
	
	//
	// Return true if the endpoint is datagram-based.
	//
	public bool datagram()
	{
	    return true;
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
	    return new UdpTransceiver(_instance, _host, _port);
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
	    UdpTransceiver p = new UdpTransceiver(_instance, _host, _port, _connect);
	    endpoint = new UdpEndpoint(_instance, _host, p.effectivePort(), _compress);
	    return p;
	}
	
	//
	// Return a connector for this endpoint, or null if no connector
	// is available.
	//
	public Connector connector()
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
	public Acceptor acceptor(ref Endpoint endpoint)
	{
	    endpoint = this;
	    return null;
	}
	
	//
	// Check whether the endpoint is equivalent to a specific
	// Transceiver or Acceptor
	//
	public bool equivalent(Transceiver transceiver)
	{
	    UdpTransceiver udpTransceiver = null;
	    try
	    {
		udpTransceiver = (UdpTransceiver) transceiver;
	    }
	    catch(System.InvalidCastException)
	    {
		return false;
	    }
	    return udpTransceiver.equivalent(_host, _port);
	}
	
	public bool equivalent(Acceptor acceptor)
	{
	    return false;
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
	    UdpEndpoint p = null;
	    
	    try
	    {
		p = (UdpEndpoint) obj;
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
	    _hashCode = 5 * _hashCode + (_connect?1:0);
	    _hashCode = 5 * _hashCode + (_compress?1:0);
	}
	
	private Instance _instance;
	private string _host;
	private int _port;
	private byte _protocolMajor;
	private byte _protocolMinor;
	private byte _encodingMajor;
	private byte _encodingMinor;
	private bool _connect;
	private bool _compress;
	private int _hashCode;
    }

    sealed class UdpEndpointFactory : EndpointFactory
    {
        internal UdpEndpointFactory(Instance instance)
        {
            _instance = instance;
        }
	
        public short type()
        {
            return UdpEndpoint.TYPE;
        }
	
        public string protocol()
        {
            return "udp";
        }
	
        public Endpoint create(string str)
        {
            return new UdpEndpoint(_instance, str);
        }
	
        public Endpoint read(BasicStream s)
        {
            return new UdpEndpoint(s);
        }
	
        public void destroy()
        {
            _instance = null;
        }
	
        private Instance _instance;
    }

}
