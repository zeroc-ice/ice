//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Net;

namespace IceInternal
{
    internal sealed class TcpEndpoint : IPEndpoint
    {
        public TcpEndpoint(ProtocolInstance instance, string ho, int po, EndPoint? sourceAddr, int ti, string conId,
                            bool co) :
            base(instance, ho, po, sourceAddr, conId)
        {
            _timeout = ti;
            _compress = co;
        }

        public TcpEndpoint(ProtocolInstance instance) :
            base(instance)
        {
            _timeout = instance.DefaultTimeout;
            _compress = false;
        }

        public TcpEndpoint(ProtocolInstance instance, Ice.InputStream s) :
            base(instance, s)
        {
            _timeout = s.ReadInt();
            _compress = s.ReadBool();
        }

        private sealed class Info : Ice.TCPEndpointInfo
        {
            public Info(IPEndpoint e) => _endpoint = e;

            public override short type() => _endpoint.type();

            public override bool datagram() => _endpoint.datagram();

            public override bool secure() => _endpoint.secure();

            private IPEndpoint _endpoint;
        }

        public override void streamWriteImpl(Ice.OutputStream s)
        {
            base.streamWriteImpl(s);
            s.WriteInt(_timeout);
            s.WriteBool(_compress);
        }

        public override Ice.EndpointInfo getInfo()
        {
            Info info = new Info(this);
            fillEndpointInfo(info);
            return info;
        }

        public override int timeout() => _timeout;

        public override Endpoint timeout(int timeout)
        {
            if (timeout == _timeout)
            {
                return this;
            }
            else
            {
                return new TcpEndpoint(instance_, host_!, port_, sourceAddr_, timeout, connectionId_, _compress);
            }
        }

        public override bool compress() => _compress;

        public override Endpoint compress(bool compress)
        {
            if (compress == _compress)
            {
                return this;
            }
            else
            {
                return new TcpEndpoint(instance_, host_!, port_, sourceAddr_, _timeout, connectionId_, compress);
            }
        }

        public override bool datagram() => false;

        public override ITransceiver? transceiver() => null;

        public override IAcceptor acceptor(string adapterName) => new TcpAcceptor(this, instance_, host_!, port_);

        public TcpEndpoint endpoint(TcpAcceptor acceptor)
        {
            int port = acceptor.effectivePort();
            if (port == port_)
            {
                return this;
            }
            else
            {
                return new TcpEndpoint(instance_, host_!, port, sourceAddr_, _timeout, connectionId_, _compress);
            }
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

            if (_timeout == -1)
            {
                s += " -t infinite";
            }
            else
            {
                s += " -t " + _timeout;
            }

            if (_compress)
            {
                s += " -z";
            }

            return s;
        }

        public override int CompareTo(Endpoint obj)
        {
            if (!(obj is TcpEndpoint))
            {
                return type() < obj.type() ? -1 : 1;
            }

            TcpEndpoint p = (TcpEndpoint)obj;
            if (this == p)
            {
                return 0;
            }

            if (_timeout < p._timeout)
            {
                return -1;
            }
            else if (p._timeout < _timeout)
            {
                return 1;
            }

            if (!_compress && p._compress)
            {
                return -1;
            }
            else if (!p._compress && _compress)
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
            if (base.checkOption(option, argument, endpoint))
            {
                return true;
            }

            switch (option[1])
            {
                case 't':
                    {
                        if (argument == null)
                        {
                            throw new FormatException($"no argument provided for -t option in endpoint {endpoint}");
                        }

                        if (argument == "infinite")
                        {
                            _timeout = -1;
                        }
                        else
                        {
                            try
                            {
                                _timeout = int.Parse(argument, CultureInfo.InvariantCulture);
                            }
                            catch (FormatException ex)
                            {
                                throw new FormatException($"invalid timeout value `{argument}' in endpoint {endpoint}", ex);
                            }

                            if (_timeout < 1)
                            {
                                throw new FormatException($"invalid timeout value `{argument}' in endpoint {endpoint}");
                            }
                        }

                        return true;
                    }

                case 'z':
                    {
                        if (argument != null)
                        {
                            throw new FormatException($"unexpected argument `{argument}' provided for -z option in {endpoint}");
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

        protected override IConnector CreateConnector(EndPoint addr, INetworkProxy? proxy) =>
            new TcpConnector(instance_, addr, proxy, sourceAddr_, _timeout, connectionId_);

        protected override IPEndpoint CreateEndpoint(string? host, int port, string connectionId) =>
            new TcpEndpoint(instance_, host!, port, sourceAddr_, _timeout, connectionId, _compress);

        private int _timeout;
        private bool _compress;
    }

    internal sealed class TcpEndpointFactory : IEndpointFactory
    {
        internal TcpEndpointFactory(ProtocolInstance instance) => _instance = instance;

        public void initialize()
        {
        }

        public short type() => _instance!.Type;

        public string protocol() => _instance!.Protocol;

        public Endpoint create(List<string> args, bool oaEndpoint)
        {
            IPEndpoint endpt = new TcpEndpoint(_instance!);
            endpt.initWithOptions(args, oaEndpoint);
            return endpt;
        }

        public Endpoint read(Ice.InputStream s) => new TcpEndpoint(_instance!, s);

        public void destroy() => _instance = null;

        public IEndpointFactory clone(ProtocolInstance instance) => new TcpEndpointFactory(instance);

        private ProtocolInstance? _instance;
    }

}
