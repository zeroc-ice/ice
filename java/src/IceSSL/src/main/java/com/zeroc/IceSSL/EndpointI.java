// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceSSL;

final class EndpointI extends com.zeroc.IceInternal.EndpointI
{
    public EndpointI(Instance instance, com.zeroc.IceInternal.EndpointI delegate)
    {
        _instance = instance;
        _delegate = delegate;
    }

    @Override
    public void streamWriteImpl(com.zeroc.Ice.OutputStream s)
    {
        _delegate.streamWriteImpl(s);
    }

    //
    // Return the endpoint information.
    //
    @Override
    public com.zeroc.Ice.EndpointInfo getInfo()
    {
        EndpointInfo info = new EndpointInfo(_delegate.getInfo(), timeout(), compress())
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
    public com.zeroc.IceInternal.EndpointI timeout(int timeout)
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
    public com.zeroc.IceInternal.EndpointI connectionId(String connectionId)
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
    public com.zeroc.IceInternal.EndpointI compress(boolean compress)
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
    public com.zeroc.IceInternal.Transceiver transceiver()
    {
        return null;
    }

    @Override
    public void connectors_async(com.zeroc.Ice.EndpointSelectionType selType,
                                 final com.zeroc.IceInternal.EndpointI_connectors callback)
    {
        com.zeroc.Ice.IPEndpointInfo ipInfo = null;
        for(com.zeroc.Ice.EndpointInfo p = _delegate.getInfo(); p != null; p = p.underlying)
        {
            if(p instanceof com.zeroc.Ice.IPEndpointInfo)
            {
                ipInfo = (com.zeroc.Ice.IPEndpointInfo)p;
            }
        }
        final String host = ipInfo != null ? ipInfo.host : "";
        com.zeroc.IceInternal.EndpointI_connectors cb = new com.zeroc.IceInternal.EndpointI_connectors()
        {
            @Override
            public void connectors(java.util.List<com.zeroc.IceInternal.Connector> connectors)
            {
                java.util.List<com.zeroc.IceInternal.Connector> l =
                    new java.util.ArrayList<com.zeroc.IceInternal.Connector>();
                for(com.zeroc.IceInternal.Connector c : connectors)
                {
                    l.add(new ConnectorI(_instance, c, host));
                }
                callback.connectors(l);
            }

            @Override
            public void exception(com.zeroc.Ice.LocalException ex)
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
    public com.zeroc.IceInternal.Acceptor acceptor(String adapterName)
    {
        return new AcceptorI(this, _instance, _delegate.acceptor(adapterName), adapterName);
    }

    public EndpointI endpoint(com.zeroc.IceInternal.EndpointI delEndpt)
    {
        return new EndpointI(_instance, delEndpt);
    }

    @Override
    public java.util.List<com.zeroc.IceInternal.EndpointI> expand()
    {
        java.util.List<com.zeroc.IceInternal.EndpointI> endps = _delegate.expand();
        java.util.List<com.zeroc.IceInternal.EndpointI> l = new java.util.ArrayList<com.zeroc.IceInternal.EndpointI>();
        for(com.zeroc.IceInternal.EndpointI e : endps)
        {
            l.add(e == _delegate ? this : new EndpointI(_instance, e));
        }
        return l;
    }

    @Override
    public boolean equivalent(com.zeroc.IceInternal.EndpointI endpoint)
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
    public int compareTo(com.zeroc.IceInternal.EndpointI obj) // From java.lang.Comparable
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
    private com.zeroc.IceInternal.EndpointI _delegate;
}
