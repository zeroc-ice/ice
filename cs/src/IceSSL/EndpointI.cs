// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System;
    using System.Diagnostics;
    using System.Collections.Generic;
    using System.Net;
    using System.Globalization;
	
    sealed class EndpointI : IceInternal.EndpointI
    {
        internal EndpointI(Instance instance, string ho, int po, int ti, string conId, bool co)
        {
            _instance = instance;
            _host = ho;
            _port = po;
            _timeout = ti;
            _connectionId = conId;
            _compress = co;
            calcHashValue();
        }

        internal EndpointI(Instance instance, string str, bool oaEndpoint)
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
                    e.str = "expected an endpoint option but found `" + option + "' in endpoint `ssl " + str + "'";
                    throw e;
                }

                string argument = null;
                if(i < arr.Length && arr[i][0] != '-')
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
                            e.str = "no argument provided for -h option in endpoint `ssl " + str + "'";
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
                            e.str = "no argument provided for -p option in endpoint `ssl " + str + "'";
                            throw e;
                        }

                        try
                        {
                            _port = System.Int32.Parse(argument, CultureInfo.InvariantCulture);
                        }
                        catch(System.FormatException ex)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
                            e.str = "invalid port value `" + argument + "' in endpoint `ssl " + str + "'";
                            throw e;
                        }

                        if(_port < 0 || _port > 65535)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "port value `" + argument + "' out of range in endpoint `ssl " + str + "'";
                            throw e;
                        }

                        break;
                    }

                    case 't':
                    {
                        if(argument == null)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "no argument provided for -t option in endpoint `ssl " + str + "'";
                            throw e;
                        }

                        try
                        {
                            _timeout = System.Int32.Parse(argument, CultureInfo.InvariantCulture);
                        }
                        catch(System.FormatException ex)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
                            e.str = "invalid timeout value `" + argument + "' in endpoint `ssl " + str + "'";
                            throw e;
                        }

                        break;
                    }

                    case 'z':
                    {
                        if(argument != null)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "unexpected argument `" + argument + "' provided for -z option in `ssl " + str +
                                    "'";
                            throw e;
                        }

                        _compress = true;
                        break;
                    }

                    default:
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException();
                        e.str = "unknown option `" + option + "' in `ssl " + str + "'";
                        throw e;
                    }
                }
            }

            if(_host == null)
            {
                _host = _instance.defaultHost();
            }
            else if(_host.Equals("*"))
            {
                if(oaEndpoint)
                {
                    _host = null;
                }
                else
                {
                    throw new Ice.EndpointParseException("`-h *' not valid for proxy endpoint `ssl " + str + "'");
                }
            }

            if(_host == null)
            {
                _host = "";
            }

            calcHashValue();
        }

        internal EndpointI(Instance instance, IceInternal.BasicStream s)
        {
            _instance = instance;
            s.startReadEncaps();
            _host = s.readString();
            _port = s.readInt();
            _timeout = s.readInt();
            _compress = s.readBool();
            s.endReadEncaps();
            calcHashValue();
        }

        //
        // Marshal the endpoint.
        //
        public override void streamWrite(IceInternal.BasicStream s)
        {
            s.writeShort(EndpointType.value);
            s.startWriteEncaps();
            s.writeString(_host);
            s.writeInt(_port);
            s.writeInt(_timeout);
            s.writeBool(_compress);
            s.endWriteEncaps();
        }

        //
        // Convert the endpoint to its string form.
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
            string s = "ssl";

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
        
        private sealed class InfoI : IceSSL.EndpointInfo
        {
            public InfoI(int to, bool comp, string host, int port) : base(to, comp, host, port)
            {
            }

            override public short type()
            {
                return EndpointType.value;
            }
            
            override public bool datagram()
            {
                return false;
            }
                
            override public bool secure()
            {
                return true;
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
        // Return the endpoint type.
        //
        public override short type()
        {
            return EndpointType.value;
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
        public override IceInternal.EndpointI timeout(int timeout)
        {
            if(timeout == _timeout)
            {
                return this;
            }
            else
            {
                return new EndpointI(_instance, _host, _port, timeout, _connectionId, _compress);
            }
        }

        //
        // Return a new endpoint with a different connection id.
        //
        public override IceInternal.EndpointI connectionId(string connectionId)
        {
            if(connectionId == _connectionId)
            {
                return this;
            }
            else
            {
                return new EndpointI(_instance, _host, _port, _timeout, connectionId, _compress);
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
        public override IceInternal.EndpointI compress(bool compress)
        {
            if(compress == _compress)
            {
                return this;
            }
            else
            {
                return new EndpointI(_instance, _host, _port, _timeout, _connectionId, compress);
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
            return true;
        }

        //
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor. In case a
        // transceiver is created, this operation also returns a new
        // "effective" endpoint, which might differ from this endpoint,
        // for example, if a dynamic port number is assigned.
        //
        public override IceInternal.Transceiver transceiver(ref IceInternal.EndpointI endpoint)
        {
            endpoint = this;
            return null;
        }

        //
        // Return connectors for this endpoint, or empty list if no connector
        // is available.
        //
        public override List<IceInternal.Connector> connectors()
        {
            return connectors(IceInternal.Network.getAddresses(_host, _port, _instance.protocolSupport()));
        }

        public override void connectors_async(IceInternal.EndpointI_connectors callback)
        {
            _instance.endpointHostResolver().resolve(_host, _port, this, callback);
        }

        //
        // Return an acceptor for this endpoint, or null if no acceptor
        // is available. In case an acceptor is created, this operation
        // also returns a new "effective" endpoint, which might differ
        // from this endpoint, for example, if a dynamic port number is
        // assigned.
        //
        public override IceInternal.Acceptor acceptor(ref IceInternal.EndpointI endpoint, string adapterName)
        {
            AcceptorI p = new AcceptorI(_instance, adapterName, _host, _port);
            endpoint = new EndpointI(_instance, _host, p.effectivePort(), _timeout, _connectionId, _compress);
            return p;
        }

        //
        // Expand endpoint out in to separate endpoints for each local
        // host if listening on INADDR_ANY.
        //
        public override List<IceInternal.EndpointI> expand()
        {
            List<IceInternal.EndpointI> endps = new List<IceInternal.EndpointI>();
            List<string> hosts = 
                IceInternal.Network.getHostsForEndpointExpand(_host, _instance.protocolSupport(), false);
            if(hosts == null || hosts.Count == 0)
            {
                endps.Add(this);
            }
            else
            {
                foreach(string h in hosts)
                {
                    endps.Add(new EndpointI(_instance, h, _port, _timeout, _connectionId, _compress));
                }
            }
            return endps;
        }

        //
        // Check whether the endpoint is equivalent to another one.
        //
        public override bool equivalent(IceInternal.EndpointI endpoint)
        {
            EndpointI sslEndpointI = null;
            try
            {
                sslEndpointI = (EndpointI)endpoint;
            }
            catch(System.InvalidCastException)
            {
                return false;
            }
            return sslEndpointI._host.Equals(_host) && sslEndpointI._port == _port;
        }

        public override List<IceInternal.Connector> connectors(List<IPEndPoint> addresses)
        {
            List<IceInternal.Connector> connectors = new List<IceInternal.Connector>();
            foreach(IPEndPoint addr in addresses)
            {
                connectors.Add(new ConnectorI(_instance, _host, addr, _timeout, _connectionId));
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
            EndpointI p = null;

            try
            {
                p = (EndpointI)obj;
            }
            catch(System.InvalidCastException)
            {
                try
                {
                    IceInternal.EndpointI e = (IceInternal.EndpointI)obj;
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

            if(_timeout < p._timeout)
            {
                return -1;
            }
            else if(p._timeout < _timeout)
            {
                return 1;
            }

            if(!_connectionId.Equals(p._connectionId))
            {
                return string.Compare(_connectionId, p._connectionId, StringComparison.Ordinal);
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
            _hashCode = _host.GetHashCode();
            _hashCode = 5 * _hashCode + _port;
            _hashCode = 5 * _hashCode + _timeout;
            _hashCode = 5 * _hashCode + _connectionId.GetHashCode();
            _hashCode = 5 * _hashCode + (_compress ? 1 : 0);
        }

        private Instance _instance;
        private string _host;
        private int _port;
        private int _timeout;
        private string _connectionId = "";
        private bool _compress;
        private int _hashCode;
    }

    internal sealed class EndpointFactoryI : IceInternal.EndpointFactory
    {
        internal EndpointFactoryI(Instance instance)
        {
            _instance = instance;
        }

        public short type()
        {
            return EndpointType.value;
        }

        public string protocol()
        {
            return "ssl";
        }

        public IceInternal.EndpointI create(string str, bool oaEndpoint)
        {
            return new EndpointI(_instance, str, oaEndpoint);
        }

        public IceInternal.EndpointI read(IceInternal.BasicStream s)
        {
            return new EndpointI(_instance, s);
        }

        public void destroy()
        {
            _instance = null;
        }

        private Instance _instance;
    }
}
