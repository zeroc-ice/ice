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
    using System.Collections.Generic;
    using System.Net;
    using System;
    using System.Globalization;

    sealed class TcpEndpointI : EndpointI
    {
        public TcpEndpointI(Instance instance, string ho, int po, int ti, string conId, bool co) : base(conId)
        {
            _instance = instance;
            _host = ho;
            _port = po;
            _timeout = ti;
            _compress = co;
            calcHashValue();
        }

        public TcpEndpointI(Instance instance, string str, bool oaEndpoint) : base("")
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
                    e.str = "expected an endpoint option but found `" + option + "' in endpoint `tcp " + str + "'";
                    throw e;
                }

                string argument = null;
                if(i < arr.Length && arr[i].Length > 0 && arr[i][0] != '-')
                {
                    argument = arr[i++];
                    if(argument[0] == '\"' && argument[argument.Length - 1] == '\"')
                    {
                        argument = argument.Substring(1, argument.Length - 2);
                    }
                }

                switch(option[1])
                {
                    case 'h':
                    {
                        if(argument == null)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "no argument provided for -h option in endpoint `tcp " + str + "'";
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
                            e.str = "no argument provided for -p option in endpoint `tcp " + str + "'";
                            throw e;
                        }

                        try
                        {
                            _port = System.Int32.Parse(argument, CultureInfo.InvariantCulture);
                        }
                        catch(System.FormatException ex)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
                            e.str = "invalid port value `" + argument + "' in endpoint `tcp " + str + "'";
                            throw e;
                        }

                        if(_port < 0 || _port > 65535)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "port value `" + argument + "' out of range in endpoint `tcp " + str + "'";
                            throw e;
                        }

                        break;
                    }

                    case 't':
                    {
                        if(argument == null)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "no argument provided for -t option in endpoint `tcp " + str + "'";
                            throw e;
                        }

                        try
                        {
                            _timeout = System.Int32.Parse(argument, CultureInfo.InvariantCulture);
                        }
                        catch(System.FormatException ex)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
                            e.str = "invalid timeout value `" + argument + "' in endpoint `tcp " + str + "'";
                            throw e;
                        }

                        break;
                    }

                    case 'z':
                    {
                        if(argument != null)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "unexpected argument `" + argument + "' provided for -z option in `tcp " + str +
                                    "'";
                            throw e;
                        }

                        _compress = true;
                        break;
                    }

                    default:
                    {
                        parseOption(option, argument, "tcp", str);
                        break;
                    }
                }
            }

            if(_host == null)
            {
                _host = _instance.defaultsAndOverrides().defaultHost;
            }
            else if(_host.Equals("*"))
            {
                if(oaEndpoint)
                {
                    _host = null;
                }
                else
                {
                    throw new Ice.EndpointParseException("`-h *' not valid for proxy endpoint `tcp " + str + "'");
                }
            }

            if(_host == null)
            {
                _host = "";
            }

            calcHashValue();
        }

        public TcpEndpointI(BasicStream s)
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
        public override void streamWrite(BasicStream s)
        {
            s.writeShort(Ice.TCPEndpointType.value);
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
        public override string ice_toString_()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            string s = "tcp";

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

        private sealed class InfoI : Ice.TCPEndpointInfo
        {
            public InfoI(int to, bool comp, string host, int port) : base(to, comp, host, port)
            {
            }

            override public short type()
            {
                return Ice.TCPEndpointType.value;
            }
            
            override public bool datagram()
            {
                return false;
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
            return new InfoI(_timeout, _compress, _host, _port);
        }

        //
        // Return the endpoint type
        //
        public override short type()
        {
            return Ice.TCPEndpointType.value;
        }

        //
        // Return the protocol name;
        //
        public override string protocol()
        {
            return "tcp";
        }

        //
        // Return the timeout for the endpoint in milliseconds. 0 means
        // non-blocking, -1 means no timeout.
        //
        public override int timeout()
        {
            return _timeout;
        }

        //
        // Return a new endpoint with a different timeout value, provided
        // that timeouts are supported by the endpoint. Otherwise the same
        // endpoint is returned.
        //
        public override EndpointI timeout(int timeout)
        {
            if(timeout == _timeout)
            {
                return this;
            }
            else
            {
                return new TcpEndpointI(_instance, _host, _port, timeout, connectionId_, _compress);
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
                return new TcpEndpointI(_instance, _host, _port, _timeout, connectionId, _compress);
            }
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
                return new TcpEndpointI(_instance, _host, _port, _timeout, connectionId_, compress);
            }
        }

        //
        // Return true if the endpoint is datagram-based.
        //
        public override bool datagram()
        {
            return false;
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
            endpoint = this;
            return null;
        }

        //
        // Return connectors for this endpoint, or empty list if no connector
        // is available.
        //
        public override List<Connector> connectors(Ice.EndpointSelectionType selType)
        {
#if SILVERLIGHT
            return connectors(Network.getAddresses(_host, 
                                                   _port,
                                                   _instance.protocolSupport(), 
                                                   selType, 
                                                   _instance.preferIPv6(), 
                                                   false),
                              _instance.networkProxy());
#else
            return _instance.endpointHostResolver().resolve(_host, _port, selType, this);
#endif
        }


        public override void connectors_async(Ice.EndpointSelectionType selType, EndpointI_connectors callback)
        {
#if SILVERLIGHT
            callback.connectors(connectors(selType));
#else
            _instance.endpointHostResolver().resolve(_host, _port, selType, this, callback);
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
#if SILVERLIGHT
            throw new Ice.FeatureNotSupportedException("server endpoint not supported for `" + ToString() + "'");
#else
            TcpAcceptor p = new TcpAcceptor(_instance, _host, _port);
            endpoint = new TcpEndpointI(_instance, _host, p.effectivePort(), _timeout, connectionId_, _compress);
            return p;
#endif
        }

        //
        // Expand endpoint out in to separate endpoints for each local
        // host if listening on INADDR_ANY.
        //
        public override List<EndpointI> expand()
        {
            List<EndpointI> endps = new List<EndpointI>();
            List<string> hosts = Network.getHostsForEndpointExpand(_host, _instance.protocolSupport(), false);
            if(hosts == null || hosts.Count == 0)
            {
                endps.Add(this);
            }
            else
            {
                foreach(string h in hosts)
                {
                    endps.Add(new TcpEndpointI(_instance, h, _port, _timeout, connectionId_, _compress));
                }
            }
            return endps;
        }

        //
        // Check whether the endpoint is equivalent to another one.
        //
        public override bool equivalent(EndpointI endpoint)
        {
            if(!(endpoint is TcpEndpointI))
            {
                return false;
            }

            TcpEndpointI tcpEndpointI = (TcpEndpointI)endpoint;
            return tcpEndpointI._host.Equals(_host) && tcpEndpointI._port == _port;
        }

        public override List<Connector> connectors(List<EndPoint> addresses, NetworkProxy networkProxy)
        {
            List<Connector> connectors = new List<Connector>();
            foreach(EndPoint addr in addresses)
            {
                connectors.Add(new TcpConnector(_instance, addr, networkProxy, _timeout, connectionId_));
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
            if(!(obj is TcpEndpointI))
            {
                return type() < obj.type() ? -1 : 1;
            }

            TcpEndpointI p = (TcpEndpointI)obj;
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

            return string.Compare(_host, p._host, StringComparison.Ordinal);
        }

        private void calcHashValue()
        {
            int h = 5381;
            IceInternal.HashUtil.hashAdd(ref h, Ice.TCPEndpointType.value);
            IceInternal.HashUtil.hashAdd(ref h, _host);
            IceInternal.HashUtil.hashAdd(ref h, _port);
            IceInternal.HashUtil.hashAdd(ref h, _timeout);
            IceInternal.HashUtil.hashAdd(ref h, connectionId_);
            IceInternal.HashUtil.hashAdd(ref h, _compress);
            _hashCode = h;
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
            return Ice.TCPEndpointType.value;
        }

        public string protocol()
        {
            return "tcp";
        }

        public EndpointI create(string str, bool oaEndpoint)
        {
            return new TcpEndpointI(_instance, str, oaEndpoint);
        }

        public EndpointI read(BasicStream s)
        {
            return new TcpEndpointI(s);
        }

        public void destroy()
        {
            _instance = null;
        }

        private Instance _instance;
    }

}
