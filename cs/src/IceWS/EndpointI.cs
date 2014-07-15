// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceWS
{
    using System;
    using System.Diagnostics;
    using System.Collections.Generic;
    using System.Globalization;

    sealed class EndpointI : IceInternal.EndpointI
    {
        internal EndpointI(Instance instance, IceInternal.EndpointI del, string res)
        {
            _instance = instance;
            _delegate = (IceInternal.IPEndpointI)del;
            _resource = res;
        }

        internal EndpointI(Instance instance, IceInternal.EndpointI del, List<string> args)
        {
            _instance = instance;
            _delegate = (IceInternal.IPEndpointI)del;

            initWithOptions(args);

            if(_resource == null)
            {
                _resource = "/";
            }
        }

        internal EndpointI(Instance instance, IceInternal.EndpointI del, IceInternal.BasicStream s)
        {
            _instance = instance;
            _delegate = (IceInternal.IPEndpointI)del;

            _resource = s.readString();
        }

        private sealed class InfoI : IceWS.EndpointInfo
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

        public override Ice.EndpointInfo getInfo()
        {
            InfoI info = new InfoI(this);
            info.timeout = _delegate.timeout();
            info.compress = _delegate.compress();
            _delegate.fillEndpointInfo(info);
            info.resource = _resource;
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

        public override void streamWrite(IceInternal.BasicStream s)
        {
            s.startWriteEncaps();
            _delegate.streamWriteImpl(s);
            s.writeString(_resource);
            s.endWriteEncaps();
        }

        public override int timeout()
        {
            return _delegate.timeout();
        }

        public override IceInternal.EndpointI timeout(int timeout)
        {
            if(timeout == _delegate.timeout())
            {
                return this;
            }
            else
            {
                return new EndpointI(_instance, _delegate.timeout(timeout), _resource);
            }
        }

        public override string connectionId()
        {
            return _delegate.connectionId();
        }

        public override IceInternal.EndpointI connectionId(string connectionId)
        {
            if(connectionId.Equals(_delegate.connectionId()))
            {
                return this;
            }
            else
            {
                return new EndpointI(_instance, _delegate.connectionId(connectionId), _resource);
            }
        }

        public override bool compress()
        {
            return _delegate.compress();
        }

        public override IceInternal.EndpointI compress(bool compress)
        {
            if(compress == _delegate.compress())
            {
                return this;
            }
            else
            {
                return new EndpointI(_instance, _delegate.compress(compress), _resource);
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

        public override IceInternal.Transceiver transceiver(ref IceInternal.EndpointI endpoint)
        {
            endpoint = this;
            return null;
        }

        public override List<IceInternal.Connector> connectors(Ice.EndpointSelectionType selType)
        {
            List<IceInternal.Connector> connectors = _delegate.connectors(selType);
            List<IceInternal.Connector> l = new List<IceInternal.Connector>();
            foreach(IceInternal.Connector c in connectors)
            {
                l.Add(new ConnectorI(_instance, c, _delegate.host(), _delegate.port(), _resource));
            }
            return l;
        }

        private sealed class EndpointI_connectorsI : IceInternal.EndpointI_connectors
        {
            public EndpointI_connectorsI(Instance instance, string host, int port, string resource,
                                         IceInternal.EndpointI_connectors cb)
            {
                _instance = instance;
                _host = host;
                _port = port;
                _resource = resource;
                _callback = cb;
            }

            public void connectors(List<IceInternal.Connector> connectors)
            {
                List<IceInternal.Connector> l = new List<IceInternal.Connector>();
                foreach(IceInternal.Connector c in connectors)
                {
                    l.Add(new ConnectorI(_instance, c, _host, _port, _resource));
                }
                _callback.connectors(l);
            }

            public void exception(Ice.LocalException ex)
            {
                _callback.exception(ex);
            }

            private Instance _instance;
            private string _host;
            private int _port;
            private string _resource;
            private IceInternal.EndpointI_connectors _callback;
        }

        public override void connectors_async(Ice.EndpointSelectionType selType,
                                              IceInternal.EndpointI_connectors callback)
        {
            EndpointI_connectorsI cb =
                new EndpointI_connectorsI(_instance, _delegate.host(), _delegate.port(), _resource, callback);
            _delegate.connectors_async(selType, cb);
        }

        public override IceInternal.Acceptor acceptor(ref IceInternal.EndpointI endpoint, string adapterName)
        {
            IceInternal.EndpointI delEndp = null;
            IceInternal.Acceptor delAcc = _delegate.acceptor(ref delEndp, adapterName);
            if(delEndp != null)
            {
                endpoint = new EndpointI(_instance, delEndp, _resource);
            }
            return new AcceptorI(_instance, delAcc);
        }

        public override List<IceInternal.EndpointI> expand()
        {
            List<IceInternal.EndpointI> endps = _delegate.expand();
            List<IceInternal.EndpointI> l = new List<IceInternal.EndpointI>();
            foreach(IceInternal.EndpointI e in endps)
            {
                l.Add(e == _delegate ? this : new EndpointI(_instance, e, _resource));
            }
            return l;
        }

        public override bool equivalent(IceInternal.EndpointI endpoint)
        {
            if(!(endpoint is EndpointI))
            {
                return false;
            }
            EndpointI wsEndpointI = (EndpointI)endpoint;
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

            if(_resource != null && _resource.Length > 0)
            {
                s += " -r ";
                bool addQuote = _resource.IndexOf(':') != -1;
                if(addQuote)
                {
                    s += "\"";
                }
                s += _resource;
                if(addQuote)
                {
                    s += "\"";
                }
            }

            return s;
        }

        public override int GetHashCode()
        {
            int h = _delegate.GetHashCode();
            IceInternal.HashUtil.hashAdd(ref h, _resource);
            return h;
        }

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

            int v = string.Compare(_resource, p._resource, StringComparison.Ordinal);
            if(v != 0)
            {
                return v;
            }

            return _delegate.CompareTo(p._delegate);
        }

        protected override bool checkOption(string option, string argument, string endpoint)
        {
            switch(option[1])
            {
            case 'r':
            {
                if(argument == null)
                {
                    Ice.EndpointParseException e = new Ice.EndpointParseException();
                    e.str = "no argument provided for -r option in endpoint " + endpoint + _delegate.options();
                    throw e;
                }
                _resource = argument;
                return true;
            }

            default:
            {
                return false;
            }
            }
        }

        private Instance _instance;
        private IceInternal.IPEndpointI _delegate;
        private string _resource;
    }

    internal sealed class EndpointFactoryI : IceInternal.EndpointFactory
    {
        internal EndpointFactoryI(Instance instance, IceInternal.EndpointFactory del)
        {
            _instance = instance;
            _delegate = del;
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
            return new EndpointI(_instance, _delegate.create(args, oaEndpoint), args);
        }

        public IceInternal.EndpointI read(IceInternal.BasicStream s)
        {
            return new EndpointI(_instance, _delegate.read(s), s);
        }

        public void destroy()
        {
            _delegate.destroy();
            _instance = null;
        }

        public IceInternal.EndpointFactory clone(IceInternal.ProtocolInstance instance)
        {
            Debug.Assert(false); // We don't support cloning this transport.
            return null;
        }

        private Instance _instance;
        private IceInternal.EndpointFactory _delegate;
    }
}
