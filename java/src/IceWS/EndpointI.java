// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceWS;

final class EndpointI extends IceInternal.EndpointI
{
    public EndpointI(Instance instance, IceInternal.EndpointI del, String res)
    {
        _instance = instance;
        _delegate = (IceInternal.IPEndpointI)del;
        _resource = res;
    }

    public EndpointI(Instance instance, IceInternal.EndpointI del, java.util.ArrayList<String> args)
    {
        _instance = instance;
        _delegate = (IceInternal.IPEndpointI)del;

        initWithOptions(args);

        if(_resource == null)
        {
            _resource = "/";
        }
    }

    public EndpointI(Instance instance, IceInternal.EndpointI del, IceInternal.BasicStream s)
    {
        _instance = instance;
        _delegate = (IceInternal.IPEndpointI)del;

        _resource = s.readString();
    }

    @Override
    public Ice.EndpointInfo getInfo()
    {
        IceWS.EndpointInfo info = new IceWS.EndpointInfo()
            {
                @Override
                public short type()
                {
                    return EndpointI.this.type();
                }

                @Override
                public boolean datagram()
                {
                    return EndpointI.this.datagram();
                }

                @Override
                public boolean secure()
                {
                    return EndpointI.this.secure();
                }
            };

        info.timeout = _delegate.timeout();
        info.compress = _delegate.compress();
        _delegate.fillEndpointInfo(info);
        info.resource = _resource;
        return info;
    }

    @Override
    public short type()
    {
        return _delegate.type();
    }

    @Override
    public String protocol()
    {
        return _delegate.protocol();
    }

    @Override
    public void streamWrite(IceInternal.BasicStream s)
    {
        s.startWriteEncaps();
        _delegate.streamWriteImpl(s);
        s.writeString(_resource);
        s.endWriteEncaps();
    }

    @Override
    public int timeout()
    {
        return _delegate.timeout();
    }

    @Override
    public IceInternal.EndpointI timeout(int timeout)
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

    @Override
    public String connectionId()
    {
        return _delegate.connectionId();
    }

    @Override
    public IceInternal.EndpointI connectionId(String connectionId)
    {
        if(connectionId.equals(_delegate.connectionId()))
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _delegate.connectionId(connectionId), _resource);
        }
    }

    @Override
    public boolean compress()
    {
        return _delegate.compress();
    }

    @Override
    public IceInternal.EndpointI compress(boolean compress)
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

    @Override
    public boolean datagram()
    {
        return _delegate.datagram();
    }

    @Override
    public boolean secure()
    {
        return _delegate.secure();
    }

    @Override
    public IceInternal.Transceiver transceiver(IceInternal.EndpointIHolder endpoint)
    {
        endpoint.value = this;
        return null;
    }

    @Override
    public java.util.List<IceInternal.Connector> connectors(Ice.EndpointSelectionType selType)
    {
        java.util.List<IceInternal.Connector> connectors = _delegate.connectors(selType);
        java.util.List<IceInternal.Connector> l = new java.util.ArrayList<IceInternal.Connector>();
        for(IceInternal.Connector c : connectors)
        {
            l.add(new ConnectorI(_instance, c, _delegate.host(), _delegate.port(), _resource));
        }
        return l;
    }

    @Override
    public void connectors_async(Ice.EndpointSelectionType selType, final IceInternal.EndpointI_connectors callback)
    {
        IceInternal.EndpointI_connectors cb = new IceInternal.EndpointI_connectors()
        {
            @Override
            public void connectors(java.util.List<IceInternal.Connector> connectors)
            {
                java.util.List<IceInternal.Connector> l = new java.util.ArrayList<IceInternal.Connector>();
                for(IceInternal.Connector c : connectors)
                {
                    l.add(new ConnectorI(_instance, c, _delegate.host(), _delegate.port(), _resource));
                }
                callback.connectors(l);
            }

            @Override
            public void exception(Ice.LocalException ex)
            {
                callback.exception(ex);
            }
        };
        _delegate.connectors_async(selType, cb);
    }

    @Override
    public IceInternal.Acceptor acceptor(IceInternal.EndpointIHolder endpoint, String adapterName)
    {
        IceInternal.EndpointIHolder delEndp = new IceInternal.EndpointIHolder();
        IceInternal.Acceptor delAcc = _delegate.acceptor(delEndp, adapterName);
        if(delEndp.value != null)
        {
            endpoint.value = new EndpointI(_instance, delEndp.value, _resource);
        }
        return new AcceptorI(_instance, delAcc);
    }

    @Override
    public java.util.List<IceInternal.EndpointI> expand()
    {
        java.util.List<IceInternal.EndpointI> endps = _delegate.expand();
        java.util.List<IceInternal.EndpointI> l = new java.util.ArrayList<IceInternal.EndpointI>();
        for(IceInternal.EndpointI e : endps)
        {
            l.add(e == _delegate ? this : new EndpointI(_instance, e, _resource));
        }
        return l;
    }

    @Override
    public boolean equivalent(IceInternal.EndpointI endpoint)
    {
        if(!(endpoint instanceof EndpointI))
        {
            return false;
        }
        EndpointI wsEndpointI = (EndpointI)endpoint;
        return _delegate.equivalent(wsEndpointI._delegate);
    }

    @Override
    synchronized public int hashCode()
    {
        int h = _delegate.hashCode();
        h = IceInternal.HashUtil.hashAdd(h, _resource);
        return h;
    }

    @Override
    public String options()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        String s = _delegate.options();

        if(_resource != null && _resource.length() > 0)
        {
            s += " -r ";
            boolean addQuote = _resource.indexOf(':') != -1;
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

    @Override
    public int compareTo(IceInternal.EndpointI obj) // From java.lang.Comparable
    {
        if(!(obj instanceof EndpointI))
        {
            return type() < obj.type() ? -1 : 1;
        }

        EndpointI p = (EndpointI)obj;
        if(this == p)
        {
            return 0;
        }

        int v = _resource.compareTo(p._resource);
        if(v != 0)
        {
            return v;
        }

        return _delegate.compareTo(p._delegate);
    }

    @Override
    protected boolean checkOption(String option, String argument, String endpoint)
    {
        switch(option.charAt(1))
        {
        case 'r':
        {
            if(argument == null)
            {
                throw new Ice.EndpointParseException("no argument provided for -r option in endpoint " + endpoint +
                    _delegate.options());
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
    private String _resource;
}
