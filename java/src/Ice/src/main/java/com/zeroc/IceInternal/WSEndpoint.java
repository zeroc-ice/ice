// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import com.zeroc.Ice.WSEndpointInfo;

final class WSEndpoint extends EndpointI
{
    public WSEndpoint(ProtocolInstance instance, EndpointI del, String res)
    {
        _instance = instance;
        _delegate = del;
        _resource = res;
    }

    public WSEndpoint(ProtocolInstance instance, EndpointI del, java.util.ArrayList<String> args)
    {
        _instance = instance;
        _delegate = del;

        initWithOptions(args);

        if(_resource == null)
        {
            _resource = "/";
        }
    }

    public WSEndpoint(ProtocolInstance instance, EndpointI del, com.zeroc.Ice.InputStream s)
    {
        _instance = instance;
        _delegate = del;
        _resource = s.readString();
    }

    @Override
    public com.zeroc.Ice.EndpointInfo getInfo()
    {
        WSEndpointInfo info = new WSEndpointInfo(_delegate.getInfo(), timeout(), compress(), _resource)
        {
            @Override
            public short type()
            {
                return WSEndpoint.this.type();
            }

            @Override
            public boolean datagram()
            {
                return WSEndpoint.this.datagram();
            }

            @Override
            public boolean secure()
            {
                return WSEndpoint.this.secure();
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
    public void streamWriteImpl(com.zeroc.Ice.OutputStream s)
    {
        _delegate.streamWriteImpl(s);
        s.writeString(_resource);
    }

    @Override
    public int timeout()
    {
        return _delegate.timeout();
    }

    @Override
    public EndpointI timeout(int timeout)
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

    @Override
    public String connectionId()
    {
        return _delegate.connectionId();
    }

    @Override
    public EndpointI connectionId(String connectionId)
    {
        if(connectionId.equals(_delegate.connectionId()))
        {
            return this;
        }
        else
        {
            return new WSEndpoint(_instance, _delegate.connectionId(connectionId), _resource);
        }
    }

    @Override
    public boolean compress()
    {
        return _delegate.compress();
    }

    @Override
    public EndpointI compress(boolean compress)
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
    public Transceiver transceiver()
    {
        return null;
    }

    @Override
    public void connectors_async(com.zeroc.Ice.EndpointSelectionType selType, final EndpointI_connectors callback)
    {
        com.zeroc.Ice.IPEndpointInfo ipInfo = null;
        for(com.zeroc.Ice.EndpointInfo p = _delegate.getInfo(); p != null; p = p.underlying)
        {
            if(p instanceof com.zeroc.Ice.IPEndpointInfo)
            {
                ipInfo = (com.zeroc.Ice.IPEndpointInfo)p;
            }
        }
        final String host = ipInfo != null ? (ipInfo.host + ":" + ipInfo.port) : "";
        EndpointI_connectors cb = new EndpointI_connectors()
        {
            @Override
            public void connectors(java.util.List<Connector> connectors)
            {
                java.util.List<Connector> l = new java.util.ArrayList<>();
                for(Connector c : connectors)
                {
                    l.add(new WSConnector(_instance, c, host, _resource));
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

    @Override
    public Acceptor acceptor(String adapterName)
    {
        Acceptor delAcc = _delegate.acceptor(adapterName);
        return new WSAcceptor(this, _instance, delAcc);
    }

    public WSEndpoint endpoint(EndpointI delEndp)
    {
        if(delEndp == _delegate)
        {
            return this;
        }
        else
        {
            return new WSEndpoint(_instance, delEndp, _resource);
        }
    }

    @Override
    public java.util.List<EndpointI> expandIfWildcard()
    {
        java.util.List<EndpointI> endps = _delegate.expandIfWildcard();
        java.util.List<EndpointI> l = new java.util.ArrayList<>();
        for(EndpointI e : endps)
        {
            l.add(e == _delegate ? this : new WSEndpoint(_instance, e, _resource));
        }
        return l;
    }

    @Override
    public EndpointI.ExpandHostResult expandHost()
    {
        EndpointI.ExpandHostResult result = _delegate.expandHost();
        java.util.List<EndpointI> l = new java.util.ArrayList<>();
        for(EndpointI e : result.endpoints)
        {
            l.add(e == _delegate ? this : new WSEndpoint(_instance, e, _resource));
        }
        result.endpoints = l;
        if(result.publish != null)
        {
            result.publish = result.publish == _delegate ? this : new WSEndpoint(_instance, result.publish, _resource);
        }
        return result;
    }

    @Override
    public boolean equivalent(EndpointI endpoint)
    {
        if(!(endpoint instanceof WSEndpoint))
        {
            return false;
        }
        WSEndpoint wsEndpointI = (WSEndpoint)endpoint;
        return _delegate.equivalent(wsEndpointI._delegate);
    }

    @Override
    synchronized public int hashCode()
    {
        int h = _delegate.hashCode();
        h = HashUtil.hashAdd(h, _resource);
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
    public int compareTo(EndpointI obj) // From java.lang.Comparable
    {
        if(!(obj instanceof WSEndpoint))
        {
            return type() < obj.type() ? -1 : 1;
        }

        WSEndpoint p = (WSEndpoint)obj;
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

    public EndpointI delegate()
    {
        return _delegate;
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
                throw new com.zeroc.Ice.EndpointParseException("no argument provided for -r option in endpoint " +
                                                               endpoint + _delegate.options());
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
    private EndpointI _delegate;
    private String _resource;
}
