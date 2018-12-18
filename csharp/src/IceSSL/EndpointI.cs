// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System.Collections.Generic;

    sealed class EndpointI : IceInternal.EndpointI
    {
        internal EndpointI(Instance instance, IceInternal.EndpointI del)
        {
            _instance = instance;
            _delegate = del;
        }

        public override void streamWriteImpl(Ice.OutputStream os)
        {
            _delegate.streamWriteImpl(os);
        }

        private sealed class InfoI : EndpointInfo
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
            info.underlying = _delegate.getInfo();
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
                return new EndpointI(_instance, _delegate.timeout(timeout));
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
                return new EndpointI(_instance, _delegate.connectionId(connectionId));
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
                return new EndpointI(_instance, _delegate.compress(compress));
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

        public override IceInternal.Transceiver transceiver()
        {
            return null;
        }

        private sealed class EndpointI_connectorsI : IceInternal.EndpointI_connectors
        {
            public EndpointI_connectorsI(Instance instance, string host, IceInternal.EndpointI_connectors cb)
            {
                _instance = instance;
                _host = host;
                _callback = cb;
            }

            public void connectors(List<IceInternal.Connector> connectors)
            {
                List<IceInternal.Connector> l = new List<IceInternal.Connector>();
                foreach(IceInternal.Connector c in connectors)
                {
                    l.Add(new ConnectorI(_instance, c, _host));
                }
                _callback.connectors(l);
            }

            public void exception(Ice.LocalException ex)
            {
                _callback.exception(ex);
            }

            private Instance _instance;
            private string _host;
            private IceInternal.EndpointI_connectors _callback;
        }

        public override void connectors_async(Ice.EndpointSelectionType selType,
                                              IceInternal.EndpointI_connectors callback)
        {
            string host = "";
            for(Ice.EndpointInfo p = _delegate.getInfo(); p != null; p = p.underlying)
            {
                if(p is Ice.IPEndpointInfo)
                {
                    host = ((Ice.IPEndpointInfo)p).host;
                    break;
                }
            }
            _delegate.connectors_async(selType, new EndpointI_connectorsI(_instance, host, callback));
        }

        public override IceInternal.Acceptor acceptor(string adapterName)
        {
            return new AcceptorI(this, _instance, _delegate.acceptor(adapterName), adapterName);
        }

        public EndpointI endpoint(IceInternal.EndpointI del)
        {
            if(del == _delegate)
            {
                return this;
            }
            else
            {
                return new EndpointI(_instance, del);
            }
        }

        public override List<IceInternal.EndpointI> expandIfWildcard()
        {
            List<IceInternal.EndpointI> l = new List<IceInternal.EndpointI>();
            foreach(IceInternal.EndpointI e in _delegate.expandIfWildcard())
            {
                l.Add(e == _delegate ? this : new EndpointI(_instance, e));
            }
            return l;
        }

        public override List<IceInternal.EndpointI> expandHost(out IceInternal.EndpointI publish)
        {
            List<IceInternal.EndpointI> l = new List<IceInternal.EndpointI>();
            foreach(IceInternal.EndpointI e in _delegate.expandHost(out publish))
            {
                l.Add(e == _delegate ? this : new EndpointI(_instance, e));
            }
            if(publish != null)
            {
                publish = publish == _delegate ? this : new EndpointI(_instance, publish);
            }
            return l;
        }

        public override bool equivalent(IceInternal.EndpointI endpoint)
        {
            if(!(endpoint is EndpointI))
            {
                return false;
            }
            EndpointI endpointI = (EndpointI)endpoint;
            return _delegate.equivalent(endpointI._delegate);
        }

        public override string options()
        {
            return _delegate.options();
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

            return _delegate.CompareTo(p._delegate);
        }

        public override int GetHashCode()
        {
            return _delegate.GetHashCode();
        }

        protected override bool checkOption(string option, string argument, string endpoint)
        {
            return false;
        }

        private Instance _instance;
        private IceInternal.EndpointI _delegate;
    }

    internal sealed class EndpointFactoryI : IceInternal.EndpointFactoryWithUnderlying
    {
        public EndpointFactoryI(Instance instance, short type) : base(instance, type)
        {
            _instance = instance;
        }

        override public IceInternal.EndpointFactory
        cloneWithUnderlying(IceInternal.ProtocolInstance inst, short underlying)
        {
            return new EndpointFactoryI(new Instance(_instance.engine(), inst.type(), inst.protocol()), underlying);
        }

        override protected IceInternal.EndpointI
        createWithUnderlying(IceInternal.EndpointI underlying, List<string> args, bool oaEndpoint)
        {
            return new EndpointI(_instance, underlying);
        }

        override protected IceInternal.EndpointI
        readWithUnderlying(IceInternal.EndpointI underlying, Ice.InputStream s)
        {
            return new EndpointI(_instance, underlying);
        }

        private Instance _instance;
    }
}
