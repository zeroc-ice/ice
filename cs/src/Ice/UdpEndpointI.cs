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
    using System.Collections;

    sealed class UdpEndpointI : EndpointI
    {
	internal const short TYPE = 3;
	
	public UdpEndpointI(Instance instance, string ho, int po, bool co, bool pub)
	{
	    instance_ = instance;
	    _host = ho;
	    _port = po;
	    _protocolMajor = Protocol.protocolMajor;
	    _protocolMinor = Protocol.protocolMinor;
	    _encodingMajor = Protocol.encodingMajor;
	    _encodingMinor = Protocol.encodingMinor;
	    _connect = false;
	    _compress = co;
	    _publish = pub;
	    calcHashValue();
	}
	
	public UdpEndpointI(Instance instance, string str, bool adapterEndp)
	{
	    instance_ = instance;
	    _host = null;
	    _port = 0;
	    _protocolMajor = Protocol.protocolMajor;
	    _protocolMinor = Protocol.protocolMinor;
	    _encodingMajor = Protocol.encodingMajor;
	    _encodingMinor = Protocol.encodingMinor;
	    _connect = false;
	    _compress = false;
	    _publish = true;
	    
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
	    else if(_host.Equals("*") && adapterEndp)
	    {
	        _host = "0.0.0.0";
	    }
	    
	    calcHashValue();
	}
	
	public UdpEndpointI(BasicStream s)
	{
	    instance_ = s.instance();
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
	    _publish = true;
	    calcHashValue();
	}
	
	//
	// Marshal the endpoint
	//
	public override void streamWrite(BasicStream s)
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
	public override string ice_toString_()
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
	public override short type()
	{
	    return TYPE;
	}
	
	//
	// Return the timeout for the endpoint in milliseconds. 0 means
	// non-blocking, -1 means no timeout.
	//
	public override int timeout()
	{
	    return -1;
	}
	
	//
	// Return true if the endpoints support bzip2 compress, or false
	// otherwise.
	//
	public override bool compress()
	{
	    return _compress;
	}
	
	//
	// Return a new endpoint with a different compression value,
	// provided that compression is supported by the
	// endpoint. Otherwise the same endpoint is returned.
	//
	public override EndpointI compress(bool compress)
	{
	    if(compress == _compress)
	    {
		return this;
	    }
	    else
	    {
		return new UdpEndpointI(instance_, _host, _port, compress, _publish);
	    }
	}
	
	//
	// Return a new endpoint with a different timeout value, provided
	// that timeouts are supported by the endpoint. Otherwise the same
	// endpoint is returned.
	//
	public override EndpointI timeout(int timeout)
	{
	    return this;
	}
	
	//
	// Return true if the endpoint is datagram-based.
	//
	public override bool datagram()
	{
	    return true;
	}
	
	//
	// Return true if the endpoint is secure.
	//
	public override bool secure()
	{
	    return false;
	}
	
	//
	// Return true if the endpoint type is unknown.
	//
	public override bool unknown()
	{
	    return false;
	}
	
	//
	// Return a client side transceiver for this endpoint, or null if a
	// transceiver can only be created by a connector.
	//
	public override Transceiver clientTransceiver()
	{
	    return new UdpTransceiver(instance_, _host, _port);
	}
	
	//
	// Return a server side transceiver for this endpoint, or null if a
	// transceiver can only be created by an acceptor. In case a
	// transceiver is created, this operation also returns a new
	// "effective" endpoint, which might differ from this endpoint,
	// for example, if a dynamic port number is assigned.
	//
	public override Transceiver serverTransceiver(ref EndpointI endpoint)
	{
	    UdpTransceiver p = new UdpTransceiver(instance_, _host, _port, _connect);
	    endpoint = new UdpEndpointI(instance_, _host, p.effectivePort(), _compress, _publish);
	    return p;
	}
	
	//
	// Return a connector for this endpoint, or null if no connector
	// is available.
	//
	public override Connector connector()
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
	public override Acceptor acceptor(ref EndpointI endpoint)
	{
	    endpoint = this;
	    return null;
	}

        //
        // Expand endpoint out in to separate endpoints for each local
        // host if endpoint was configured with no host set. This
        // only applies for ObjectAdapter endpoints.
        //
        public override ArrayList
        expand()
        {
            ArrayList endps = new ArrayList();
            if(_host.Equals("0.0.0.0"))
            {
                string[] hosts = Network.getLocalHosts();
                for(int i = 0; i < hosts.Length; ++i)
                {
                    endps.Add(new UdpEndpointI(instance_, hosts[i], _port, _compress,
                                               hosts.Length == 1 || !hosts[i].Equals("127.0.0.1")));
                }
            }
            else
            {
                endps.Add(this);
            }
            return endps;
        }

        //
        // Return whether endpoint should be published in proxies
        // created by Object Adapter.
        //
        public override bool
        publish()
        {
            return _publish;
        }
	
	//
	// Check whether the endpoint is equivalent to a specific
	// Transceiver or Acceptor
	//
	public override bool equivalent(Transceiver transceiver)
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
	
	public override bool equivalent(Acceptor acceptor)
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
	
	public override int CompareTo(object obj)
	{
	    UdpEndpointI p = null;
	    
	    try
	    {
		p = (UdpEndpointI) obj;
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
	
	private Instance instance_;
	private string _host;
	private int _port;
	private byte _protocolMajor;
	private byte _protocolMinor;
	private byte _encodingMajor;
	private byte _encodingMinor;
	private bool _connect;
	private bool _compress;
	private bool _publish;
	private int _hashCode;
    }

    sealed class UdpEndpointFactory : EndpointFactory
    {
        internal UdpEndpointFactory(Instance instance)
        {
            instance_ = instance;
        }
	
        public short type()
        {
            return UdpEndpointI.TYPE;
        }
	
        public string protocol()
        {
            return "udp";
        }
	
        public EndpointI create(string str, bool adapterEndp)
        {
            return new UdpEndpointI(instance_, str, adapterEndp);
        }
	
        public EndpointI read(BasicStream s)
        {
            return new UdpEndpointI(s);
        }
	
        public void destroy()
        {
            instance_ = null;
        }
	
        private Instance instance_;
    }

}
