//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace IceInternal
{
    internal sealed class WSEndpoint : EndpointI
    {
        internal WSEndpoint(ProtocolInstance instance, EndpointI del, string res)
        {
            _instance = instance;
            _delegate = del;
            _resource = res;
        }

#pragma warning disable CS8618 // Non-nullable field is uninitialized. Consider declaring as nullable.
        internal WSEndpoint(ProtocolInstance instance, EndpointI del, List<string> args)
#pragma warning restore CS8618 // Non-nullable field is uninitialized. Consider declaring as nullable.
        {
            _instance = instance;
            _delegate = del;

            initWithOptions(args);

            if (_resource == null)
            {
                _resource = "/";
            }
        }

        internal WSEndpoint(ProtocolInstance instance, EndpointI del, Ice.InputStream s)
        {
            _instance = instance;
            _delegate = del;

            _resource = s.ReadString();
        }

        private sealed class InfoI : Ice.WSEndpointInfo
        {
            public InfoI(EndpointI e)
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

            private readonly EndpointI _endpoint;
        }

        public override Ice.EndpointInfo getInfo()
        {
            Ice.WSEndpointInfo info = new InfoI(this);
            info.underlying = _delegate.getInfo();
            info.resource = _resource;
            info.compress = info.underlying.compress;
            info.timeout = info.underlying.timeout;
            return info;
        }

        public override short type()
        {
            return _delegate.type();
        }

        public override string protocol()
        {
            return _delegate.protocol();
        }

        public override void streamWriteImpl(Ice.OutputStream s)
        {
            _delegate.streamWriteImpl(s);
            s.WriteString(_resource);
        }

        public override int timeout()
        {
            return _delegate.timeout();
        }

        public override EndpointI timeout(int timeout)
        {
            if (timeout == _delegate.timeout())
            {
                return this;
            }
            else
            {
                return new WSEndpoint(_instance, _delegate.timeout(timeout), _resource);
            }
        }

        public override string connectionId()
        {
            return _delegate.connectionId();
        }

        public override EndpointI connectionId(string connectionId)
        {
            if (connectionId.Equals(_delegate.connectionId()))
            {
                return this;
            }
            else
            {
                return new WSEndpoint(_instance, _delegate.connectionId(connectionId), _resource);
            }
        }

        public override bool compress()
        {
            return _delegate.compress();
        }

        public override EndpointI compress(bool compress)
        {
            if (compress == _delegate.compress())
            {
                return this;
            }
            else
            {
                return new WSEndpoint(_instance, _delegate.compress(compress), _resource);
            }
        }

        public override bool datagram()
        {
            return _delegate.datagram();
        }

        public override bool secure()
        {
            return _delegate.secure();
        }

        public override Transceiver? transceiver()
        {
            return null;
        }

        private sealed class EndpointI_connectorsI : EndpointI_connectors
        {
            public EndpointI_connectorsI(ProtocolInstance instance, string host, string res, EndpointI_connectors cb)
            {
                _instance = instance;
                _host = host;
                _resource = res;
                _callback = cb;
            }

            public void connectors(List<Connector> connectors)
            {
                List<Connector> l = new List<Connector>();
                foreach (Connector c in connectors)
                {
                    l.Add(new WSConnector(_instance, c, _host, _resource));
                }
                _callback.connectors(l);
            }

            public void exception(Ice.LocalException ex)
            {
                _callback.exception(ex);
            }

            private readonly ProtocolInstance _instance;
            private readonly string _host;
            private readonly string _resource;
            private readonly EndpointI_connectors _callback;
        }

        public override void connectors_async(Ice.EndpointSelectionType selType, EndpointI_connectors callback)
        {
            string host = "";
            for (Ice.EndpointInfo p = _delegate.getInfo(); p != null; p = p.underlying)
            {
                if (p is Ice.IPEndpointInfo ipInfo)
                {
                    host = ipInfo.host + ":" + ipInfo.port;
                    break;
                }
            }
            _delegate.connectors_async(selType, new EndpointI_connectorsI(_instance, host, _resource, callback));
        }

        public override Acceptor acceptor(string adapterName)
        {
            var acceptor = _delegate.acceptor(adapterName);
            Debug.Assert(acceptor != null);
            return new WSAcceptor(this, _instance, acceptor);
        }

        public WSEndpoint endpoint(EndpointI delEndp)
        {
            if (delEndp == _delegate)
            {
                return this;
            }
            else
            {
                return new WSEndpoint(_instance, delEndp, _resource);
            }
        }

        public override List<EndpointI> expandIfWildcard()
        {
            List<EndpointI> l = new List<EndpointI>();
            foreach (EndpointI e in _delegate.expandIfWildcard())
            {
                l.Add(e == _delegate ? this : new WSEndpoint(_instance, e, _resource));
            }
            return l;
        }

        public override List<EndpointI> expandHost(out EndpointI? publish)
        {
            List<EndpointI> l = new List<EndpointI>();
            foreach (EndpointI e in _delegate.expandHost(out publish))
            {
                l.Add(e == _delegate ? this : new WSEndpoint(_instance, e, _resource));
            }
            if (publish != null)
            {
                publish = publish == _delegate ? this : new WSEndpoint(_instance, publish, _resource);
            }
            return l;
        }

        public override bool equivalent(EndpointI endpoint)
        {
            if (!(endpoint is WSEndpoint))
            {
                return false;
            }
            WSEndpoint wsEndpointI = (WSEndpoint)endpoint;
            return _delegate.equivalent(wsEndpointI._delegate);
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
            string s = _delegate.options();

            if (_resource != null && _resource.Length > 0)
            {
                s += " -r ";
                bool addQuote = _resource.IndexOf(':') != -1;
                if (addQuote)
                {
                    s += "\"";
                }
                s += _resource;
                if (addQuote)
                {
                    s += "\"";
                }
            }

            return s;
        }

        public override int GetHashCode()
        {
            int h = _delegate.GetHashCode();
            HashUtil.hashAdd(ref h, _resource);
            return h;
        }

        public override int CompareTo(EndpointI obj)
        {
            if (!(obj is WSEndpoint))
            {
                return type() < obj.type() ? -1 : 1;
            }

            WSEndpoint p = (WSEndpoint)obj;
            if (this == p)
            {
                return 0;
            }

            int v = string.Compare(_resource, p._resource, StringComparison.Ordinal);
            if (v != 0)
            {
                return v;
            }

            return _delegate.CompareTo(p._delegate);
        }

        protected override bool checkOption(string option, string? argument, string endpoint)
        {
            switch (option[1])
            {
                case 'r':
                {
                    _resource = argument ?? throw new FormatException(
                            $"no argument provided for -r option in endpoint {endpoint}{_delegate.options()}");
                    return true;
                }
                default:
                {
                    return false;
                }
            }
        }

        private readonly ProtocolInstance _instance;
        private readonly EndpointI _delegate;
        private string _resource;
    }

    public class WSEndpointFactory : EndpointFactoryWithUnderlying
    {
        public WSEndpointFactory(ProtocolInstance instance, short type) : base(instance, type)
        {
        }

        public override EndpointFactory cloneWithUnderlying(ProtocolInstance instance, short underlying)
        {
            return new WSEndpointFactory(instance, underlying);
        }

        protected override EndpointI createWithUnderlying(EndpointI underlying, List<string> args, bool oaEndpoint)
        {
            return new WSEndpoint(instance_, underlying, args);
        }

        protected override EndpointI readWithUnderlying(EndpointI underlying, Ice.InputStream s)
        {
            return new WSEndpoint(instance_, underlying, s);
        }
    }
}
