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

            public override short Type() => _endpoint.Type();

            public override bool Datagram() => _endpoint.Datagram();

            public override bool Secure() => _endpoint.Secure();

            private readonly IPEndpoint _endpoint;
        }

        public override void StreamWriteImpl(Ice.OutputStream s)
        {
            base.StreamWriteImpl(s);
            s.WriteInt(_timeout);
            s.WriteBool(_compress);
        }

        public override Ice.EndpointInfo GetInfo()
        {
            var info = new Info(this);
            FillEndpointInfo(info);
            return info;
        }

        public override int Timeout() => _timeout;

        public override Endpoint Timeout(int timeout)
        {
            if (timeout == _timeout)
            {
                return this;
            }
            else
            {
                return new TcpEndpoint(Instance, Host!, Port, SourceAddr, timeout, ConnectionId_, _compress);
            }
        }

        public override bool Compress() => _compress;

        public override Endpoint Compress(bool compress)
        {
            if (compress == _compress)
            {
                return this;
            }
            else
            {
                return new TcpEndpoint(Instance, Host!, Port, SourceAddr, _timeout, ConnectionId_, compress);
            }
        }

        public override bool Datagram() => false;

        public override ITransceiver? Transceiver() => null;

        public override IAcceptor Acceptor(string adapterName) => new TcpAcceptor(this, Instance, Host!, Port);

        public TcpEndpoint Endpoint(TcpAcceptor acceptor)
        {
            int port = acceptor.EffectivePort();
            if (port == Port)
            {
                return this;
            }
            else
            {
                return new TcpEndpoint(Instance, Host!, port, SourceAddr, _timeout, ConnectionId_, _compress);
            }
        }

        public override string Options()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            string s = base.Options();

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
                return Type() < obj.Type() ? -1 : 1;
            }

            var p = (TcpEndpoint)obj;
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

        public override void HashInit(ref HashCode hash)
        {
            hash.Add(_timeout);
            hash.Add(_compress);
        }

        public override void FillEndpointInfo(Ice.IPEndpointInfo info)
        {
            base.FillEndpointInfo(info);
            info.Timeout = _timeout;
            info.Compress = _compress;
        }

        protected override bool CheckOption(string option, string argument, string endpoint)
        {
            if (base.CheckOption(option, argument, endpoint))
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
            new TcpConnector(Instance, addr, proxy, SourceAddr, _timeout, ConnectionId_);

        protected override IPEndpoint CreateEndpoint(string? host, int port, string connectionId) =>
            new TcpEndpoint(Instance, host!, port, SourceAddr, _timeout, connectionId, _compress);

        private int _timeout;
        private bool _compress;
    }

    internal sealed class TcpEndpointFactory : IEndpointFactory
    {
        internal TcpEndpointFactory(ProtocolInstance instance) => _instance = instance;

        public void Initialize()
        {
        }

        public short Type() => _instance!.Type;

        public string Protocol() => _instance!.Protocol;

        public Endpoint Create(List<string> args, bool oaEndpoint)
        {
            IPEndpoint endpt = new TcpEndpoint(_instance!);
            endpt.InitWithOptions(args, oaEndpoint);
            return endpt;
        }

        public Endpoint Read(Ice.InputStream s) => new TcpEndpoint(_instance!, s);

        public void Destroy() => _instance = null;

        public IEndpointFactory Clone(ProtocolInstance instance) => new TcpEndpointFactory(instance);

        private ProtocolInstance? _instance;
    }

}
