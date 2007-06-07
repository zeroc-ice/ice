// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        
        public UdpEndpointI(Instance instance, string ho, int po, string mif, int mttl, bool conn, string conId,
                            bool co, bool oae)
        {
            instance_ = instance;
            _host = ho;
            _port = po;
            _mcastInterface = mif;
            _mcastTtl = mttl;
            _protocolMajor = Protocol.protocolMajor;
            _protocolMinor = Protocol.protocolMinor;
            _encodingMajor = Protocol.encodingMajor;
            _encodingMinor = Protocol.encodingMinor;
            _connect = conn;
            _connectionId = conId;
            _compress = co;
            _oaEndpoint = oae;
            calcHashValue();
        }
        
        public UdpEndpointI(Instance instance, string str, bool oaEndpoint)
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
            _oaEndpoint = true;
            
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
                if(option[0] != '-')
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
                
                if(option.Equals("-v"))
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
                }
                else if(option.Equals("-e"))
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
                }
                else if(option.Equals("-h"))
                {
                    if(argument == null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "udp " + str;
                        throw e;
                    }
                    
                    _host = argument;
                }
                else if(option.Equals("-p"))
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

                    if(_port < 0 || _port > 65535)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "udp " + str;
                        throw e;
                    }
                }
                else if(option.Equals("-c"))
                {
                    if(argument != null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "udp " + str;
                        throw e;
                    }
                    
                    _connect = true;
                }
                else if(option.Equals("-z"))
                {
                    if(argument != null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "udp " + str;
                        throw e;
                    }
                    
                    _compress = true;
                }
                else if(option.Equals("--interface"))
                {
                    if(argument == null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "udp " + str;
                        throw e;
                    }
                    
                    _mcastInterface = argument;
                }
                else if(option.Equals("--ttl"))
                {
                    if(argument == null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "udp " + str;
                        throw e;
                    }
                    
                    try
                    {
                        _mcastTtl = System.Int32.Parse(argument);
                    }
                    catch(System.FormatException ex)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
                        e.str = "udp " + str;
                        throw e;
                    }

                    if(_mcastTtl < 0)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "udp " + str;
                        throw e;
                    }
                }
                else
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "udp " + str;
                    throw e;
                }
            }

            if(_host == null)
            {
                _host = instance_.defaultsAndOverrides().defaultHost;
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
            else if(_host.Equals("*"))
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
            _oaEndpoint = false;
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
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
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

            if(_mcastInterface.Length != 0)
            {
                s += " --interface " + _mcastInterface;
            }

            if(_mcastTtl != -1)
            {
                s += " --ttl " + _mcastTtl;
            }

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
                return new UdpEndpointI(instance_, _host, _port, _mcastInterface, _mcastTtl, _connect, _connectionId,
                                        compress, _oaEndpoint);
            }
        }

        //
        // Return a new endpoint with a different connection id.
        //
        public override EndpointI connectionId(string connectionId)
        {
            if(connectionId == _connectionId)
            {
                return this;
            }
            else
            {
                return new UdpEndpointI(instance_, _host, _port, _mcastInterface, _mcastTtl, _connect, connectionId,
                                        _compress, _oaEndpoint);
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
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor. In case a
        // transceiver is created, this operation also returns a new
        // "effective" endpoint, which might differ from this endpoint,
        // for example, if a dynamic port number is assigned.
        //
        public override Transceiver transceiver(ref EndpointI endpoint)
        {
            UdpTransceiver p = new UdpTransceiver(instance_, _host, _port, _mcastInterface, _connect);
            endpoint = new UdpEndpointI(instance_, _host, p.effectivePort(), _mcastInterface, _mcastTtl, _connect,
                                        _connectionId, _compress, _oaEndpoint);
            return p;
        }
        
        //
        // Return a connector for this endpoint, or empty list if no connector
        // is available.
        //
        public override ArrayList connectors()
        {
            ArrayList connectors = new ArrayList();
            System.Net.IPEndPoint[] addresses = Network.getAddresses(_host, _port);
            for(int i = 0; i < addresses.Length; ++i)
            {
                connectors.Add(new UdpConnector(instance_, addresses[i], _mcastInterface, _mcastTtl, _protocolMajor,
                                                _protocolMinor, _encodingMajor, _encodingMinor, _connectionId));
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
        public override Acceptor acceptor(ref EndpointI endpoint, string adapterName)
        {
            endpoint = this;
            return null;
        }

        //
        // Expand endpoint out in to separate endpoints for each local
        // host if endpoint was configured with no host set.
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
                    if(!_oaEndpoint || hosts.Length == 1 || !hosts[i].Equals("127.0.0.1"))
                    {
                        endps.Add(new UdpEndpointI(instance_, hosts[i], _port, _mcastInterface, _mcastTtl, _connect, 
                                                   _connectionId, _compress, _oaEndpoint));
                    }
                }
            }
            else
            {
                endps.Add(this);
            }
            return endps;
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

        public override bool requiresThreadPerConnection()
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
                try
                {
                    EndpointI e = (EndpointI)obj;
                    return type() < e.type() ? -1 : 1;
                }
                catch(System.InvalidCastException)
                {
                    Debug.Assert(false);
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
            
            if(!_connect && p._connect)
            {
                return -1;
            }
            else if(!p._connect && _connect)
            {
                return 1;
            }
            
            if(!_connectionId.Equals(p._connectionId))
            {
                return _connectionId.CompareTo(p._connectionId);
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

            int rc = _mcastInterface.CompareTo(p._mcastInterface);
            if(rc != 0)
            {
                return rc;
            }
            
            if(_mcastTtl < p._mcastTtl)
            {
                return -1;
            }
            else if(p._mcastTtl < _mcastTtl)
            {
                return 1;
            }

            return _host.CompareTo(p._host);
        }
        
        private void calcHashValue()
        {
            try
            {
                _hashCode = Network.getNumericHost(_host).GetHashCode();
            }
            catch(Ice.DNSException)
            {
                _hashCode = _host.GetHashCode();
            }
            _hashCode = 5 * _hashCode + _port;
            _hashCode = 5 * _hashCode + _mcastInterface.GetHashCode();
            _hashCode = 5 * _hashCode + _mcastTtl.GetHashCode();
            _hashCode = 5 * _hashCode + (_connect?1:0);
            _hashCode = 5 * _hashCode + _connectionId.GetHashCode();
            _hashCode = 5 * _hashCode + (_compress?1:0);
        }
        
        private Instance instance_;
        private string _host;
        private int _port;
        private string _mcastInterface = "";
        private int _mcastTtl = -1;
        private byte _protocolMajor;
        private byte _protocolMinor;
        private byte _encodingMajor;
        private byte _encodingMinor;
        private bool _connect;
        private string _connectionId = "";
        private bool _compress;
        private bool _oaEndpoint;
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
        
        public EndpointI create(string str, bool oaEndpoint)
        {
            return new UdpEndpointI(instance_, str, oaEndpoint);
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
