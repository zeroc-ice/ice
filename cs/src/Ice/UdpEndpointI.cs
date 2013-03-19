// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
    using System;
    using System.Globalization;

    sealed class UdpEndpointI : EndpointI
    {
        public UdpEndpointI(Instance instance, string ho, int po, string mif, int mttl, bool conn, string conId, 
                            bool co) : base(conId)
        {
            instance_ = instance;
            _host = ho;
            _port = po;
            _mcastInterface = mif;
            _mcastTtl = mttl;
            _connect = conn;
            connectionId_ = conId;
            _compress = co;
            calcHashValue();
        }
        
        public UdpEndpointI(Instance instance, string str, bool oaEndpoint) : base("")
        {
            instance_ = instance;
            _host = null;
            _port = 0;
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
                    e.str = "expected an endpoint option but found `" + option + "' in endpoint `udp " + str + "'";
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
                            e.str = "mismatched quotes around `" + argument + "' in endpoint `udp " + str + "'";
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
                
                if(option.Equals("-h"))
                {
                    if(argument == null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "no argument provided for -h option in endpoint `udp " + str + "'";
                        throw e;
                    }
                    
                    _host = argument;
                }
                else if(option.Equals("-p"))
                {
                    if(argument == null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "no argument provided for -p option in endpoint `udp " + str + "'";
                        throw e;
                    }
                    
                    try
                    {
                        _port = System.Int32.Parse(argument, CultureInfo.InvariantCulture);
                    }
                    catch(System.FormatException ex)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
                        e.str = "invalid port value `" + argument + "' in endpoint `udp " + str + "'";
                        throw e;
                    }

                    if(_port < 0 || _port > 65535)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "port value `" + argument + "' out of range in endpoint `udp " + str + "'";
                        throw e;
                    }
                }
                else if(option.Equals("-c"))
                {
                    if(argument != null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "unexpected argument `" + argument + "' provided for -c option in `udp " + str + "'";
                        throw e;
                    }
                    
                    _connect = true;
                }
                else if(option.Equals("-z"))
                {
                    if(argument != null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "unexpected argument `" + argument + "' provided for -z option in `udp " + str + "'";
                        throw e;
                    }
                    
                    _compress = true;
                }
                else if(option.Equals("-v") || option.Equals("-e"))
                {
                    if(argument == null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "no argument provided for " + option + " option in endpoint " + "`udp " + str + "'";
                        throw e;
                    }

                    try
                    {
                        Ice.EncodingVersion v = Ice.Util.stringToEncodingVersion(argument);
                        if(v.major != 1 || v.minor != 0)
                        {
                            instance_.initializationData().logger.warning("deprecated udp endpoint option: " + option);
                        }
                    }
                    catch(Ice.VersionParseException ex)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "invalid version `" + argument + "' in endpoint `udp " + str + "':\n" + ex.str;
                        throw e;
                    }
                }
                else if(option.Equals("--interface"))
                {
                    if(argument == null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "no argument provided for --interface option in endpoint `udp " + str + "'";
                        throw e;
                    }
                    
                    _mcastInterface = argument;
                }
                else if(option.Equals("--ttl"))
                {
                    if(argument == null)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "no argument provided for --ttl option in endpoint `udp " + str + "'";
                        throw e;
                    }
                    
                    try
                    {
                        _mcastTtl = System.Int32.Parse(argument, CultureInfo.InvariantCulture);
                    }
                    catch(System.FormatException ex)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
                        e.str = "invalid TTL value `" + argument + "' in endpoint `udp " + str + "'";
                        throw e;
                    }

                    if(_mcastTtl < 0)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "TTL value `" + argument + "' out of range in endpoint `udp " + str + "'";
                        throw e;
                    }
                }
                else
                {
                    parseOption(option, argument, "udp", str);
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
                    throw new Ice.EndpointParseException("`-h *' not valid for proxy endpoint `udp " + str + "'");
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
            if(s.getReadEncoding().Equals(Ice.Util.Encoding_1_0))
            {
                s.readByte();
                s.readByte();
                s.readByte();
                s.readByte();
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
            if(s.getWriteEncoding().Equals(Ice.Util.Encoding_1_0))
            {
                Ice.Util.Protocol_1_0.write__(s);
                Ice.Util.Encoding_1_0.write__(s);
            }
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
            public InfoI(bool comp, string host, int port, string mcastInterface, int mcastTtl) :
                base(-1, comp, host, port, mcastInterface, mcastTtl)
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
            return new InfoI(_compress, _host, _port, _mcastInterface, _mcastTtl);
        }

        //
        // Return the endpoint type
        //
        public override short type()
        {
            return Ice.UDPEndpointType.value;
        }
        
        //
        // Return the protocol name;
        //
        public override string protocol()
        {
            return "udp";
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
                return new UdpEndpointI(instance_, _host, _port, _mcastInterface, _mcastTtl, _connect, connectionId_, 
                                        compress);
            }
        }

        //
        // Return a new endpoint with a different connection id.
        //
        public override EndpointI connectionId(string connectionId)
        {
            if(connectionId == connectionId_)
            {
                return this;
            }
            else
            {
                return new UdpEndpointI(instance_, _host, _port, _mcastInterface, _mcastTtl, _connect, connectionId, 
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
                                        _connect, connectionId_, _compress);
            return p;
        }

        //
        // Return a connector for this endpoint, or empty list if no connector
        // is available.
        //
        public override List<Connector> connectors(Ice.EndpointSelectionType selType)
        {
            return connectors(Network.getAddresses(_host, _port, instance_.protocolSupport(), selType,
                                                   instance_.preferIPv6(), true),
                              instance_.networkProxy());
        }


        public override void connectors_async(Ice.EndpointSelectionType selType, EndpointI_connectors callback)
        {
#if SILVERLIGHT
            callback.connectors(connectors(selType));
#else
            instance_.endpointHostResolver().resolve(_host, _port, selType, this, callback);
#endif
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
                    endps.Add(new UdpEndpointI(instance_, h, _port, _mcastInterface, _mcastTtl, _connect, 
                                               connectionId_, _compress));
                }
            }
            return endps;
        }

        //
        // Check whether the endpoint is equivalent to another one.
        //
        public override bool equivalent(EndpointI endpoint)
        {
            if(!(endpoint is UdpEndpointI))
            {
                return false;
            }

            UdpEndpointI udpEndpointI = (UdpEndpointI)endpoint;
            return udpEndpointI._host.Equals(_host) && udpEndpointI._port == _port;
        }

        public override List<Connector> connectors(List<EndPoint> addresses, NetworkProxy networkProxy)
        {
            List<Connector> connectors = new List<Connector>();
            foreach(EndPoint addr in addresses)
            {
                connectors.Add(new UdpConnector(instance_, addr, _mcastInterface, _mcastTtl, connectionId_));
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
        public override int CompareTo(EndpointI obj)
        {
            if(!(obj is UdpEndpointI))
            {
                return type() < obj.type() ? -1 : 1;
            }

            UdpEndpointI p = (UdpEndpointI)obj;
            if(this == p)
            {
                return 0;
            }
            else
            {
                int r = base.CompareTo(p);
                if(r != 0)
                {
                    return r;
                }
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
            
            if(!connectionId_.Equals(p.connectionId_))
            {
                return string.Compare(connectionId_, p.connectionId_, StringComparison.Ordinal);
            }

            if(!_compress && p._compress)
            {
                return -1;
            }
            else if(!p._compress && _compress)
            {
                return 1;
            }
            
            int rc = string.Compare(_mcastInterface, p._mcastInterface, StringComparison.Ordinal);
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

            return string.Compare(_host, p._host, StringComparison.Ordinal);
        }
        
        private void calcHashValue()
        {
            int h = 5381;
            IceInternal.HashUtil.hashAdd(ref h, Ice.UDPEndpointType.value);
            IceInternal.HashUtil.hashAdd(ref h, _host);
            IceInternal.HashUtil.hashAdd(ref h, _port);
            IceInternal.HashUtil.hashAdd(ref h, _mcastInterface);
            IceInternal.HashUtil.hashAdd(ref h, _mcastTtl);
            IceInternal.HashUtil.hashAdd(ref h, _connect);
            IceInternal.HashUtil.hashAdd(ref h, connectionId_);
            IceInternal.HashUtil.hashAdd(ref h, _compress);
            _hashCode = h;
        }
        
        private Instance instance_;
        private string _host;
        private int _port;
        private string _mcastInterface = "";
        private int _mcastTtl = -1;
        private bool _connect;
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
