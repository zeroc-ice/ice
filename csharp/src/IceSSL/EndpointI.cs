// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

    sealed class EndpointI : IceInternal.IPEndpointI, IceInternal.WSEndpointDelegate
    {
        internal EndpointI(Instance instance, string ho, int po, EndPoint sourceAddr, int ti, string conId, bool co) :
            base(instance, ho, po, sourceAddr, conId)
        {
            _instance = instance;
            _timeout = ti;
            _compress = co;
        }

        internal EndpointI(Instance instance) :
            base(instance)
        {
            _instance = instance;
            _timeout = instance.defaultTimeout();
            _compress = false;
        }

        internal EndpointI(Instance instance, IceInternal.BasicStream s) :
            base(instance, s)
        {
            _instance = instance;
            _timeout = s.readInt();
            _compress = s.readBool();
        }

        private sealed class InfoI : IceSSL.EndpointInfo
        {
            public InfoI(EndpointI e)
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

            private EndpointI _endpoint;
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

        private sealed class WSSInfoI : IceSSL.WSSEndpointInfo
        {
            public WSSInfoI(EndpointI e)
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

            private EndpointI _endpoint;
        }

        //
        // Return the endpoint information.
        //
        public Ice.EndpointInfo getWSInfo(string resource)
        {
            WSSInfoI info = new WSSInfoI(this);
            fillEndpointInfo(info);
            info.resource = resource;
            return info;
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
                return new EndpointI(_instance, host_, port_, sourceAddr_, timeout, connectionId_, _compress);
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
                return new EndpointI(_instance, host_, port_, sourceAddr_, _timeout, connectionId_, compress);
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
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor.
        //
        public override IceInternal.Transceiver transceiver()
        {
            return null;
        }

        //
        // Return an acceptor for this endpoint, or null if no acceptor
        // is available.
        //
        public override IceInternal.Acceptor acceptor(string adapterName)
        {
            return new AcceptorI(this, _instance, adapterName, host_, port_);
        }

        public EndpointI endpoint(AcceptorI acceptor)
        {
            return new EndpointI(_instance, host_, acceptor.effectivePort(), sourceAddr_, _timeout, connectionId_,
                                 _compress);
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

            if(_timeout == -1)
            {
                s += " -t infinite";
            }
            else
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
        // Compare endpoints for sorting purposes
        //
        public override int CompareTo(IceInternal.EndpointI obj)
        {
            if(!(obj is EndpointI))
            {
                return type() < obj.type() ? -1 : 1;
            }

            EndpointI p = (EndpointI)obj;
            if(this == p)
            {
                return 0;
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

            return base.CompareTo(p);
        }

        public override void streamWriteImpl(IceInternal.BasicStream s)
        {
            base.streamWriteImpl(s);
            s.writeInt(_timeout);
            s.writeBool(_compress);
        }

        public override void hashInit(ref int h)
        {
            base.hashInit(ref h);
            IceInternal.HashUtil.hashAdd(ref h, _timeout);
            IceInternal.HashUtil.hashAdd(ref h, _compress);
        }

        public override void fillEndpointInfo(Ice.IPEndpointInfo info)
        {
            base.fillEndpointInfo(info);
            info.timeout = _timeout;
            info.compress = _compress;
        }

        protected override bool checkOption(string option, string argument, string endpoint)
        {
            if(base.checkOption(option, argument, endpoint))
            {
                return true;
            }

            switch(option[1])
            {
            case 't':
            {
                if(argument == null)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "no argument provided for -t option in endpoint " + endpoint;
                    throw e;
                }

                if(argument.Equals("infinite"))
                {
                    _timeout = -1;
                }
                else
                {
                    try
                    {
                        _timeout = System.Int32.Parse(argument, CultureInfo.InvariantCulture);
                        if(_timeout < 1)
                        {
                            Ice.EndpointParseException e = new Ice.EndpointParseException();
                            e.str = "invalid timeout value `" + argument + "' in endpoint " + endpoint;
                            throw e;
                        }
                    }
                    catch(System.FormatException ex)
                    {
                        Ice.EndpointParseException e = new Ice.EndpointParseException(ex);
                        e.str = "invalid timeout value `" + argument + "' in endpoint " + endpoint;
                        throw e;
                    }
                }

                return true;
            }

            case 'z':
            {
                if(argument != null)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "unexpected argument `" + argument + "' provided for -z option in " + endpoint;
                    throw e;
                }

                _compress = true;
                return true;
            }

            default:
            {
                return false;
            }
            }
        }

        protected override IceInternal.Connector createConnector(EndPoint addr, IceInternal.NetworkProxy proxy)
        {
            return new ConnectorI(_instance, host_, addr, proxy, sourceAddr_, _timeout, connectionId_);
        }

        protected override IceInternal.IPEndpointI createEndpoint(string host, int port, string connectionId)
        {
            return new EndpointI(_instance, host, port, sourceAddr_, _timeout, connectionId, _compress);
        }

        private Instance _instance;
        private int _timeout;
        private bool _compress;
    }

    internal sealed class EndpointFactoryI : IceInternal.EndpointFactory
    {
        internal EndpointFactoryI(Instance instance)
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

        public IceInternal.EndpointI create(List<string> args, bool oaEndpoint)
        {
            IceInternal.IPEndpointI endpt = new EndpointI(_instance);
            endpt.initWithOptions(args, oaEndpoint);
            return endpt;
        }

        public IceInternal.EndpointI read(IceInternal.BasicStream s)
        {
            return new EndpointI(_instance, s);
        }

        public void destroy()
        {
            _instance = null;
        }

        public IceInternal.EndpointFactory clone(IceInternal.ProtocolInstance instance)
        {
            return new EndpointFactoryI(new Instance(_instance.engine(), instance.type(), instance.protocol()));
        }

        private Instance _instance;
    }
}
