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
    using System.Collections.Generic;
    using System.Net;
    using System.Globalization;

    sealed class TcpEndpointI : IPEndpointI
    {
        public TcpEndpointI(ProtocolInstance instance, string ho, int po, EndPoint sourceAddr, int ti, string conId,
                            bool co) :
            base(instance, ho, po, sourceAddr, conId)
        {
            _timeout = ti;
            _compress = co;
        }

        public TcpEndpointI(ProtocolInstance instance) :
            base(instance)
        {
            _timeout = instance.defaultTimeout();
            _compress = false;
        }

        public TcpEndpointI(ProtocolInstance instance, Ice.InputStream s) :
            base(instance, s)
        {
            _timeout = s.readInt();
            _compress = s.readBool();
        }

        private sealed class InfoI : Ice.TCPEndpointInfo
        {
            public InfoI(IPEndpointI e)
            {
                _endpoint = e;
            }

            public override short type()
            {
                return _endpoint.type();
            }

            public override bool datagram()
            {
                return _endpoint.datagram();
            }

            public override bool secure()
            {
                return _endpoint.secure();
            }

            private IPEndpointI _endpoint;
        }

        public override void streamWriteImpl(Ice.OutputStream s)
        {
            base.streamWriteImpl(s);
            s.writeInt(_timeout);
            s.writeBool(_compress);
        }

        public override Ice.EndpointInfo getInfo()
        {
            InfoI info = new InfoI(this);
            fillEndpointInfo(info);
            return info;
        }

        public override int timeout()
        {
            return _timeout;
        }

        public override EndpointI timeout(int timeout)
        {
            if(timeout == _timeout)
            {
                return this;
            }
            else
            {
                return new TcpEndpointI(instance_, host_, port_, sourceAddr_, timeout, connectionId_, _compress);
            }
        }

        public override bool compress()
        {
            return _compress;
        }

        public override EndpointI compress(bool compress)
        {
            if(compress == _compress)
            {
                return this;
            }
            else
            {
                return new TcpEndpointI(instance_, host_, port_, sourceAddr_, _timeout, connectionId_, compress);
            }
        }

        public override bool datagram()
        {
            return false;
        }

        public override Transceiver transceiver()
        {
            return null;
        }

        public override Acceptor acceptor(string adapterName)
        {
            return new TcpAcceptor(this, instance_, host_, port_);
        }

        public TcpEndpointI endpoint(TcpAcceptor acceptor)
        {
            return new TcpEndpointI(instance_, host_, acceptor.effectivePort(), sourceAddr_, _timeout, connectionId_,
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

        public override void hashInit(ref int h)
        {
            base.hashInit(ref h);
            HashUtil.hashAdd(ref h, _timeout);
            HashUtil.hashAdd(ref h, _compress);
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
                        throw new Ice.EndpointParseException("no argument provided for -t option in endpoint " +
                                                             endpoint);
                    }

                    if(argument.Equals("infinite"))
                    {
                        _timeout = -1;
                    }
                    else
                    {
                        try
                        {
                            _timeout = int.Parse(argument, CultureInfo.InvariantCulture);
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
                        throw new Ice.EndpointParseException("unexpected argument `" + argument +
                                                             "' provided for -z option in " + endpoint);
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

        protected override Connector createConnector(EndPoint addr, NetworkProxy proxy)
        {
            return new TcpConnector(instance_, addr, proxy, sourceAddr_, _timeout, connectionId_);
        }

        protected override IPEndpointI createEndpoint(string host, int port, string connectionId)
        {
            return new TcpEndpointI(instance_, host, port, sourceAddr_, _timeout, connectionId, _compress);
        }

        private int _timeout;
        private bool _compress;
    }

    sealed class TcpEndpointFactory : EndpointFactory
    {
        internal TcpEndpointFactory(ProtocolInstance instance)
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
            IPEndpointI endpt = new TcpEndpointI(_instance);
            endpt.initWithOptions(args, oaEndpoint);
            return endpt;
        }

        public EndpointI read(Ice.InputStream s)
        {
            return new TcpEndpointI(_instance, s);
        }

        public void destroy()
        {
            _instance = null;
        }

        public EndpointFactory clone(ProtocolInstance instance, EndpointFactory del)
        {
            return new TcpEndpointFactory(instance);
        }

        private ProtocolInstance _instance;
    }

}
