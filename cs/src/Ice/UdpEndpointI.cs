// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Diagnostics;
    using System.Collections;
    using System.Collections.Generic;
    using System.Net;

    sealed class UdpEndpointI : EndpointI
    {
        public UdpEndpointI(Instance instance, string ho, int po, string mif, int mttl, byte pma, byte pmi, byte ema,
                            byte emi, bool conn, string conId, bool co)
        {
            instance_ = instance;
            _host = ho;
            _port = po;
            _mcastInterface = mif;
            _mcastTtl = mttl;
            _protocolMajor = pma;
            _protocolMinor = pmi;
            _encodingMajor = ema;
            _encodingMinor = emi;
            _connect = conn;
            _connectionId = conId;
            _compress = co;
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

            string delim = " \t\n\r";

            int beg;
            int end = 0;

            while(true)
            {
                beg = IceUtilInternal.StringUtil.findFirstNotOf(str, delim, end);
                if(beg == -1)
                {
                    break;
                }

                end = IceUtilInternal.StringUtil.findFirstOf(str, delim, beg);
                if(end == -1)
                {
                    end = str.Length;
                }

                string option = str.Substring(beg, end - beg);
                if(option[0] != '-')
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "udp " + str;
                    throw e;
                }

                string argument = null;
                int argumentBeg = IceUtilInternal.StringUtil.findFirstNotOf(str, delim, end);
                if(argumentBeg != -1 && str[argumentBeg] != '-')
                {
                    beg = argumentBeg;
                    if(str[beg] == '\"')
                    {
                        end = IceUtilInternal.StringUtil.findFirstOf(str, "\"", beg + 1);
                        if(end == -1)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "udp " + str;
                            throw e;
                        }
                        else
                        {
                            ++end;
                        }
                    }
                    else
                    {
                        end = IceUtilInternal.StringUtil.findFirstOf(str, delim, beg);
                        if(end == -1)
                        {
                            end = str.Length;
                        }
                    }
                    argument = str.Substring(beg, end - beg);
                    if(argument[0] == '\"' && argument[argument.Length - 1] == '\"')
                    {
                        argument = argument.Substring(1, argument.Length - 2);
                    }
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
            }
            else if(_host.Equals("*"))
            {
                if(oaEndpoint)
                {
                    _host = null;
                }
                else
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }        
            }

            if(_host == null)
            {
                _host = "";
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
            calcHashValue();
        }
        
        //
        // Marshal the endpoint
        //
        public override void streamWrite(BasicStream s)
        {
            s.writeShort(Ice.UDPEndpointType.value);
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

            if(_host != null && _host.Length != 0)
            {
                s += " -h ";
                bool addQuote = _host.IndexOf(':') != -1;
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
        
        private sealed class InfoI : Ice.UDPEndpointInfo
        {
            public InfoI(bool comp, string host, int port, byte protocolMajor, byte protocolMinor, 
                         byte encodingMajor, byte encodingMinor, string mcastInterface, int mcastTtl) :
                base(-1, comp, host, port, protocolMajor, protocolMinor, encodingMajor, encodingMinor, mcastInterface,
                    mcastTtl)
            {
            }

            override public short type()
            {
                return Ice.UDPEndpointType.value;
            }
                
            override public bool datagram()
            {
                return true;
            }
                
            override public bool secure()
            {
                return false;
            }
        };

        //
        // Return the endpoint information.
        //
        public override Ice.EndpointInfo getInfo()
        {
            return new InfoI(_compress, _host, _port, _protocolMajor, _protocolMinor, _encodingMajor, 
                             _encodingMinor, _mcastInterface, _mcastTtl);
        }

        //
        // Return the endpoint type
        //
        public override short type()
        {
            return Ice.UDPEndpointType.value;
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
                return new UdpEndpointI(instance_, _host, _port, _mcastInterface, _mcastTtl, _protocolMajor, 
                                        _protocolMinor, _encodingMajor, _encodingMinor, _connect, _connectionId,
                                        compress);
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
                return new UdpEndpointI(instance_, _host, _port, _mcastInterface, _mcastTtl, _protocolMajor, 
                                        _protocolMinor, _encodingMajor, _encodingMinor, _connect, connectionId,
                                        _compress);
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
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor. In case a
        // transceiver is created, this operation also returns a new
        // "effective" endpoint, which might differ from this endpoint,
        // for example, if a dynamic port number is assigned.
        //
        public override Transceiver transceiver(ref EndpointI endpoint)
        {
            UdpTransceiver p = new UdpTransceiver(instance_, _host, _port, _mcastInterface, _connect);
            endpoint = new UdpEndpointI(instance_, _host, p.effectivePort(), _mcastInterface, _mcastTtl, 
                                        _protocolMajor, _protocolMinor, _encodingMajor, _encodingMinor, _connect,
                                        _connectionId, _compress);
            return p;
        }

        //
        // Return a connector for this endpoint, or empty list if no connector
        // is available.
        //
        public override List<Connector> connectors()
        {
            return connectors(Network.getAddresses(_host, _port, instance_.protocolSupport()));
        }

        public override void connectors_async(EndpointI_connectors callback)
        {
            instance_.endpointHostResolver().resolve(_host, _port, this, callback);
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
        // host if listening on INADDR_ANY.
        //
        public override List<EndpointI>
        expand()
        {
            List<EndpointI> endps = new List<EndpointI>();
            List<string> hosts = Network.getHostsForEndpointExpand(_host, instance_.protocolSupport(), false);
            if(hosts == null || hosts.Count == 0)
            {
                endps.Add(this);
            }
            else
            {
                foreach(string h in hosts)
                {
                    endps.Add(new UdpEndpointI(instance_, h, _port, _mcastInterface, _mcastTtl, _protocolMajor, 
                                               _protocolMinor, _encodingMajor, _encodingMinor, _connect, _connectionId, 
                                               _compress));
                }
            }
            return endps;
        }

        //
        // Check whether the endpoint is equivalent to another one.
        //
        public override bool equivalent(EndpointI endpoint)
        {
            UdpEndpointI udpEndpointI = null;
            try
            {
                udpEndpointI = (UdpEndpointI)endpoint;
            }
            catch(System.InvalidCastException)
            {
                return false;
            }
            return udpEndpointI._host.Equals(_host) && udpEndpointI._port == _port;
        }

        public override List<Connector> connectors(List<IPEndPoint> addresses)
        {
            List<Connector> connectors = new List<Connector>();
            foreach(IPEndPoint addr in addresses)
            {
                connectors.Add(new UdpConnector(instance_, addr, _mcastInterface, _mcastTtl, _protocolMajor,
                                                _protocolMinor, _encodingMajor, _encodingMinor, _connectionId));
            }
            return connectors;
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
            _hashCode = _host.GetHashCode();
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
            return Ice.UDPEndpointType.value;
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
