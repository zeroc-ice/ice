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
    using System;
    using System.Diagnostics;
    using System.Collections.Generic;

    //
    // Delegate interface implemented by TcpEndpoint or IceSSL.EndpointI or any endpoint that WS can
    // delegate to.
    //
    public interface WSEndpointDelegate
    {
        Ice.EndpointInfo getWSInfo(string resource);
    }

    sealed class WSEndpoint : EndpointI
    {
        internal WSEndpoint(ProtocolInstance instance, EndpointI del, string res)
        {
            _instance = instance;
            _delegate = (IPEndpointI)del;
            _resource = res;
        }

        internal WSEndpoint(ProtocolInstance instance, EndpointI del, List<string> args)
        {
            _instance = instance;
            _delegate = (IPEndpointI)del;

            initWithOptions(args);

            if(_resource == null)
            {
                _resource = "/";
            }
        }

        internal WSEndpoint(ProtocolInstance instance, EndpointI del, Ice.InputStream s)
        {
            _instance = instance;
            _delegate = (IPEndpointI)del;

            _resource = s.readString();
        }

        private sealed class InfoI : Ice.WSEndpointInfo
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
            Debug.Assert(_delegate is WSEndpointDelegate);
            return ((WSEndpointDelegate)_delegate).getWSInfo(_resource);
        }

        public override short type()
        {
            return _delegate.type();
        }

        public override string protocol()
        {
            return _delegate.protocol();
        }

        public override void streamWrite(Ice.OutputStream s)
        {
            s.startEncapsulation();
            _delegate.streamWriteImpl(s);
            s.writeString(_resource);
            s.endEncapsulation();
        }

        public override int timeout()
        {
            return _delegate.timeout();
        }

        public override EndpointI timeout(int timeout)
        {
            if(timeout == _delegate.timeout())
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
            if(connectionId.Equals(_delegate.connectionId()))
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
            if(compress == _delegate.compress())
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

        public override Transceiver transceiver()
        {
            return null;
        }

        private sealed class EndpointI_connectorsI : EndpointI_connectors
        {
            public EndpointI_connectorsI(ProtocolInstance instance, string host, int port, string resource,
                                         EndpointI_connectors cb)
            {
                _instance = instance;
                _host = host;
                _port = port;
                _resource = resource;
                _callback = cb;
            }

            public void connectors(List<Connector> connectors)
            {
                List<Connector> l = new List<Connector>();
                foreach(Connector c in connectors)
                {
                    l.Add(new WSConnector(_instance, c, _host, _port, _resource));
                }
                _callback.connectors(l);
            }

            public void exception(Ice.LocalException ex)
            {
                _callback.exception(ex);
            }

            private ProtocolInstance _instance;
            private string _host;
            private int _port;
            private string _resource;
            private EndpointI_connectors _callback;
        }

        public override void connectors_async(Ice.EndpointSelectionType selType,
                                              EndpointI_connectors callback)
        {
            EndpointI_connectorsI cb =
                new EndpointI_connectorsI(_instance, _delegate.host(), _delegate.port(), _resource, callback);
            _delegate.connectors_async(selType, cb);
        }

        public override Acceptor acceptor(string adapterName)
        {
            Acceptor delAcc = _delegate.acceptor(adapterName);
            return new WSAcceptor(this, _instance, delAcc);
        }

        public WSEndpoint endpoint(EndpointI delEndp)
        {
            return new WSEndpoint(_instance, delEndp, _resource);
        }

        public override List<EndpointI> expand()
        {
            List<EndpointI> endps = _delegate.expand();
            List<EndpointI> l = new List<EndpointI>();
            foreach(EndpointI e in endps)
            {
                l.Add(e == _delegate ? this : new WSEndpoint(_instance, e, _resource));
            }
            return l;
        }

        public override bool equivalent(EndpointI endpoint)
        {
            if(!(endpoint is WSEndpoint))
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
            HashUtil.hashAdd(ref h, _resource);
            return h;
        }

        public override int CompareTo(EndpointI obj)
        {
            if(!(obj is EndpointI))
            {
                return type() < obj.type() ? -1 : 1;
            }

            WSEndpoint p = (WSEndpoint)obj;
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

        public EndpointI getDelegate()
        {
            return _delegate;
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

        private ProtocolInstance _instance;
        private IPEndpointI _delegate;
        private string _resource;
    }

    public class WSEndpointFactory : EndpointFactory
    {
        public WSEndpointFactory(ProtocolInstance instance, EndpointFactory del)
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

        public EndpointI create(List<string> args, bool oaEndpoint)
        {
            return new WSEndpoint(_instance, _delegate.create(args, oaEndpoint), args);
        }

        public EndpointI read(Ice.InputStream s)
        {
            return new WSEndpoint(_instance, _delegate.read(s), s);
        }

        public void destroy()
        {
            _delegate.destroy();
            _instance = null;
        }

        public EndpointFactory clone(ProtocolInstance instance)
        {
            Debug.Assert(false); // We don't support cloning this transport.
            return null;
        }

        private ProtocolInstance _instance;
        private EndpointFactory _delegate;
    }
}
