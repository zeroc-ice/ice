// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class EndpointI extends IceInternal.EndpointI
{
    public EndpointI(Instance instance, IceInternal.EndpointI delegate)
    {
        _instance = instance;
        _delegate = delegate;
    }

    @Override
    public void streamWriteImpl(Ice.OutputStream s)
    {
        _delegate.streamWriteImpl(s);
    }

    //
    // Return the endpoint information.
    //
    @Override
    public Ice.EndpointInfo getInfo()
    {
        IceSSL.EndpointInfo info = new IceSSL.EndpointInfo(_delegate.getInfo(), timeout(), compress())
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
        info.compress = info.underlying.compress;
        info.timeout = info.underlying.timeout;
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
            return new EndpointI(_instance, _delegate.timeout(timeout));
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
        if(connectionId == _delegate.connectionId())
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _delegate.connectionId(connectionId));
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
            return new EndpointI(_instance, _delegate.compress(compress));
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

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor.
    //
    @Override
    public IceInternal.Transceiver transceiver()
    {
        return null;
    }

    @Override
    public void connectors_async(Ice.EndpointSelectionType selType, final IceInternal.EndpointI_connectors callback)
    {
        Ice.IPEndpointInfo ipInfo = null;
        for(Ice.EndpointInfo p = _delegate.getInfo(); p != null; p = p.underlying)
        {
            if(p instanceof Ice.IPEndpointInfo)
            {
                ipInfo = (Ice.IPEndpointInfo)p;
            }
        }
        final String host = ipInfo != null ? ipInfo.host : "";
        IceInternal.EndpointI_connectors cb = new IceInternal.EndpointI_connectors()
        {
            @Override
            public void connectors(java.util.List<IceInternal.Connector> connectors)
            {
                java.util.List<IceInternal.Connector> l = new java.util.ArrayList<IceInternal.Connector>();
                for(IceInternal.Connector c : connectors)
                {
                    l.add(new ConnectorI(_instance, c, host));
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

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available.
    //
    @Override
    public IceInternal.Acceptor acceptor(String adapterName)
    {
        return new AcceptorI(this, _instance, _delegate.acceptor(adapterName), adapterName);
    }

    public EndpointI endpoint(IceInternal.EndpointI delEndpt)
    {
        if(delEndpt == _delegate)
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, delEndpt);
        }
    }

    @Override
    public java.util.List<IceInternal.EndpointI> expandIfWildcard()
    {
        java.util.List<IceInternal.EndpointI> l = new java.util.ArrayList<IceInternal.EndpointI>();
        for(IceInternal.EndpointI e : _delegate.expandIfWildcard())
        {
            l.add(e == _delegate ? this : new EndpointI(_instance, e));
        }
        return l;
    }

    @Override
    public java.util.List<IceInternal.EndpointI> expandHost(Ice.Holder<IceInternal.EndpointI> publish)
    {
        java.util.List<IceInternal.EndpointI> l = new java.util.ArrayList<IceInternal.EndpointI>();
        for(IceInternal.EndpointI e : _delegate.expandHost(publish))
        {
            l.add(e == _delegate ? this : new EndpointI(_instance, e));
        }
        if(publish.value != null)
        {
            publish.value = publish.value == _delegate ? this : new EndpointI(_instance, publish.value);
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
        EndpointI endpointI = (EndpointI)endpoint;
        return _delegate.equivalent(endpointI._delegate);
    }

    @Override
    synchronized public int hashCode()
    {
        return _delegate.hashCode();
    }

    @Override
    public String options()
    {
        return _delegate.options();
    }

    //
    // Compare endpoints for sorting purposes
    //
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

        return _delegate.compareTo(p._delegate);
    }

    @Override
    protected boolean checkOption(String option, String argument, String endpoint)
    {
        return false;
    }

    private Instance _instance;
    private IceInternal.EndpointI _delegate;
}
