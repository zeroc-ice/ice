// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    sealed class UdpEndpointI : IPEndpointI
    {
        public UdpEndpointI(ProtocolInstance instance, string ho, int po, EndPoint sourceAddr, string mcastInterface,
                            int mttl, bool conn, string conId, bool co) :
            base(instance, ho, po, sourceAddr, conId)
        {
            _mcastInterface = mcastInterface;
            _mcastTtl = mttl;
            _connect = conn;
            _compress = co;
        }

        public UdpEndpointI(ProtocolInstance instance) :
            base(instance)
        {
            _connect = false;
            _compress = false;
        }

        public UdpEndpointI(ProtocolInstance instance, Ice.InputStream s) :
            base(instance, s)
        {
            if(s.getEncoding().Equals(Ice.Util.Encoding_1_0))
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
        }

        private sealed class InfoI : Ice.UDPEndpointInfo
        {
            public InfoI(UdpEndpointI e)
            {
                _endpoint = e;
            }

            override public short type()
            {
                return _endpoint.type();
            }

            override public bool datagram()
            {
                return _endpoint.datagram();
            }

            override public bool secure()
            {
                return _endpoint.secure();
            }

            private UdpEndpointI _endpoint;
        }

        //
        // Return the endpoint information.
        //
        public override Ice.EndpointInfo getInfo()
        {
            InfoI info = new InfoI(this);
            fillEndpointInfo(info);
            return info;
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
        // Return a new endpoint with a different timeout value, provided
        // that timeouts are supported by the endpoint. Otherwise the same
        // endpoint is returned.
        //
        public override EndpointI timeout(int timeout)
        {
            return this;
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
                return new UdpEndpointI(instance_, host_, port_, sourceAddr_, _mcastInterface, _mcastTtl, _connect,
                                        connectionId_, compress);
            }
        }

        //
        // Return true if the endpoint is datagram-based.
        //
        public override bool datagram()
        {
            return true;
        }

        //
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor.
        //
        public override Transceiver transceiver()
        {
            return new UdpTransceiver(this, instance_, host_, port_, _mcastInterface, _connect);
        }

        //
        // Return an acceptor for this endpoint, or null if no acceptors
        // is available.
        //
        public override Acceptor acceptor(string adapterName)
        {
            return null;
        }

        public override void initWithOptions(List<string> args, bool oaEndpoint)
        {
            base.initWithOptions(args, oaEndpoint);

            if(_mcastInterface.Equals("*"))
            {
                if(oaEndpoint)
                {
                    _mcastInterface = "";
                }
                else
                {
                    throw new Ice.EndpointParseException("`--interface *' not valid for proxy endpoint `" +
                                                         ToString() + "'");
                }
            }
        }

        public UdpEndpointI endpoint(UdpTransceiver transceiver)
        {
            return new UdpEndpointI(instance_, host_, transceiver.effectivePort(), sourceAddr_, _mcastInterface,
                                    _mcastTtl, _connect, connectionId_, _compress);
        }

        public override string options()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            string s = base.options();

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

            return base.CompareTo(p);
        }

        //
        // Marshal the endpoint
        //
        public override void streamWriteImpl(Ice.OutputStream s)
        {
            base.streamWriteImpl(s);
            if(s.getEncoding().Equals(Ice.Util.Encoding_1_0))
            {
                Ice.Util.Protocol_1_0.ice_writeMembers(s);
                Ice.Util.Encoding_1_0.ice_writeMembers(s);
            }
            // Not transmitted.
            //s.writeBool(_connect);
            s.writeBool(_compress);
        }

        public override void hashInit(ref int h)
        {
            base.hashInit(ref h);
            HashUtil.hashAdd(ref h, _mcastInterface);
            HashUtil.hashAdd(ref h, _mcastTtl);
            HashUtil.hashAdd(ref h, _connect);
            HashUtil.hashAdd(ref h, _compress);
        }

        public override void fillEndpointInfo(Ice.IPEndpointInfo info)
        {
            base.fillEndpointInfo(info);
            if(info is Ice.UDPEndpointInfo)
            {
                Ice.UDPEndpointInfo udpInfo = (Ice.UDPEndpointInfo)info;
                udpInfo.timeout = -1;
                udpInfo.compress = _compress;
                udpInfo.mcastInterface = _mcastInterface;
                udpInfo.mcastTtl = _mcastTtl;
            }
        }

        protected override bool checkOption(string option, string argument, string endpoint)
        {
            if(base.checkOption(option, argument, endpoint))
            {
                return true;
            }

            if(option.Equals("-c"))
            {
                if(argument != null)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "unexpected argument `" + argument + "' provided for -c option in " + endpoint;
                    throw e;
                }

                _connect = true;
            }
            else if(option.Equals("-z"))
            {
                if(argument != null)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "unexpected argument `" + argument + "' provided for -z option in " + endpoint;
                    throw e;
                }

                _compress = true;
            }
            else if(option.Equals("-v") || option.Equals("-e"))
            {
                if(argument == null)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "no argument provided for " + option + " option in endpoint " + endpoint;
                    throw e;
                }

                try
                {
                    Ice.EncodingVersion v = Ice.Util.stringToEncodingVersion(argument);
                    if(v.major != 1 || v.minor != 0)
                    {
                        instance_.logger().warning("deprecated udp endpoint option: " + option);
                    }
                }
                catch(Ice.VersionParseException ex)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "invalid version `" + argument + "' in endpoint " + endpoint + ":\n" + ex.str;
                    throw e;
                }
            }
            else if(option.Equals("--ttl"))
            {
                if(argument == null)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "no argument provided for --ttl option in endpoint " + endpoint;
                    throw e;
                }

                try
                {
                    _mcastTtl = int.Parse(argument, CultureInfo.InvariantCulture);
                }
                catch(FormatException ex)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
                    e.str = "invalid TTL value `" + argument + "' in endpoint " + endpoint;
                    throw e;
                }

                if(_mcastTtl < 0)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "TTL value `" + argument + "' out of range in endpoint " + endpoint;
                    throw e;
                }
            }
            else if(option.Equals("--interface"))
            {
                if(argument == null)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "no argument provided for --interface option in endpoint " + endpoint;
                    throw e;
                }
                _mcastInterface = argument;
            }
            else
            {
                return false;
            }

            return true;
        }

        protected override Connector createConnector(EndPoint addr, NetworkProxy proxy)
        {
            return new UdpConnector(instance_, addr, sourceAddr_, _mcastInterface, _mcastTtl, connectionId_);
        }

        protected override IPEndpointI createEndpoint(string host, int port, string connectionId)
        {
            return new UdpEndpointI(instance_, host, port, sourceAddr_, _mcastInterface, _mcastTtl, _connect,
                                    connectionId, _compress);
        }

        private string _mcastInterface = "";
        private int _mcastTtl = -1;
        private bool _connect;
        private bool _compress;
    }

    sealed class UdpEndpointFactory : EndpointFactory
    {
        internal UdpEndpointFactory(ProtocolInstance instance)
        {
            _instance = instance;
        }

        public short type()
        {
            return _instance.type();
        }

        public string protocol()
        {
            return _instance.protocol();
        }

        public EndpointI create(List<string> args, bool oaEndpoint)
        {
            IPEndpointI endpt = new UdpEndpointI(_instance);
            endpt.initWithOptions(args, oaEndpoint);
            return endpt;
        }

        public EndpointI read(Ice.InputStream s)
        {
            return new UdpEndpointI(_instance, s);
        }

        public void destroy()
        {
            _instance = null;
        }

        public EndpointFactory clone(ProtocolInstance instance, EndpointFactory del)
        {
            return new UdpEndpointFactory(instance);
        }

        private ProtocolInstance _instance;
    }

}
