// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package test.Ice.background;

final class EndpointI extends IceInternal.EndpointI
{
    final static short TYPE_BASE = 100;

    public
    EndpointI(Configuration configuration, IceInternal.EndpointI endpoint)
    {
        _endpoint = endpoint;
        _configuration = configuration;
    }

    @Override
    public String
    _toString()
    {
        return "test-" + _endpoint.toString();
    }

    @Override
    public Ice.EndpointInfo
    getInfo()
    {
        return _endpoint.getInfo();
    }

    @Override
    public void
    streamWriteImpl(Ice.OutputStream s)
    {
        s.writeShort(_endpoint.type());
        _endpoint.streamWrite(s);
    }

    @Override
    public short
    type()
    {
        return (short)(TYPE_BASE + _endpoint.type());
    }

    @Override
    public String
    protocol()
    {
        return _endpoint.protocol();
    }

    @Override
    public int
    timeout()
    {
        return _endpoint.timeout();
    }

    @Override
    public IceInternal.EndpointI
    timeout(int timeout)
    {
        IceInternal.EndpointI endpoint = _endpoint.timeout(timeout);
        if(endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new EndpointI(_configuration, endpoint);
        }
    }

    @Override
    public String
    connectionId()
    {
        return _endpoint.connectionId();
    }

    @Override
    public IceInternal.EndpointI
    connectionId(String connectionId)
    {
        IceInternal.EndpointI endpoint = _endpoint.connectionId(connectionId);
        if(endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new EndpointI(_configuration, endpoint);
        }
    }

    @Override
    public boolean
    compress()
    {
        return _endpoint.compress();
    }

    @Override
    public IceInternal.EndpointI
    compress(boolean compress)
    {
        IceInternal.EndpointI endpoint = _endpoint.compress(compress);
        if(endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new EndpointI(_configuration, endpoint);
        }
    }

    @Override
    public boolean
    datagram()
    {
        return _endpoint.datagram();
    }

    @Override
    public boolean
    secure()
    {
        return _endpoint.secure();
    }

    @Override
    public IceInternal.Transceiver
    transceiver()
    {
        IceInternal.Transceiver transceiver = _endpoint.transceiver();
        if(transceiver != null)
        {
            return new Transceiver(_configuration, transceiver);
        }
        else
        {
            return null;
        }
    }

    @Override
    public void
    connectors_async(Ice.EndpointSelectionType selType, final IceInternal.EndpointI_connectors cb)
    {
        class Callback implements IceInternal.EndpointI_connectors
        {
            @Override
            public void
            connectors(java.util.List<IceInternal.Connector> cons)
            {
                java.util.List<IceInternal.Connector> connectors = new java.util.ArrayList<IceInternal.Connector>();
                for(IceInternal.Connector p : cons)
                {
                    connectors.add(new Connector(_configuration, p));
                }
                cb.connectors(connectors);
            }

            @Override
            public void
            exception(Ice.LocalException exception)
            {
                cb.exception(exception);
            }
        }

        try
        {
            _configuration.checkConnectorsException();
            _endpoint.connectors_async(selType, new Callback());
        }
        catch(Ice.LocalException ex)
        {
            cb.exception(ex);
        }
    }

    @Override
    public IceInternal.Acceptor
    acceptor(String adapterName)
    {
        return new Acceptor(this, _configuration, _endpoint.acceptor(adapterName));
    }

    public EndpointI
    endpoint(IceInternal.EndpointI delEndp)
    {
        if(delEndp == _endpoint)
        {
            return this;
        }
        else
        {
            return new EndpointI(_configuration, delEndp);
        }
    }

    @Override
    public java.util.List<IceInternal.EndpointI> expandIfWildcard()
    {
        java.util.List<IceInternal.EndpointI> endps = new java.util.ArrayList<IceInternal.EndpointI>();
        for(IceInternal.EndpointI endpt : _endpoint.expandIfWildcard())
        {
            endps.add(endpt == _endpoint ? this : new EndpointI(_configuration, endpt));
        }
        return endps;
    }

    @Override
    public java.util.List<IceInternal.EndpointI> expandHost(Ice.Holder<IceInternal.EndpointI> publish)
    {
        java.util.List<IceInternal.EndpointI> endps = new java.util.ArrayList<IceInternal.EndpointI>();
        for(IceInternal.EndpointI endpt : _endpoint.expandHost(publish))
        {
            endps.add(endpt == _endpoint ? this : new EndpointI(_configuration, endpt));
        }
        if(publish.value != null)
        {
            publish.value = publish.value == _endpoint ? this : new EndpointI(_configuration, publish.value);
        }
        return endps;
    }

    @Override
    public boolean
    equivalent(IceInternal.EndpointI endpoint)
    {
        EndpointI testEndpoint = null;
        try
        {
            testEndpoint = (EndpointI)endpoint;
        }
        catch(ClassCastException ex)
        {
            return false;
        }
        return testEndpoint._endpoint.equivalent(_endpoint);
    }

    @Override
    public String
    options()
    {
        return _endpoint.options();
    }

    @Override
    public int
    hashCode()
    {
        return _endpoint.hashCode();
    }

    //
    // Compare endpoints for sorting purposes
    //
    @Override
    public boolean
    equals(java.lang.Object obj)
    {
        try
        {
            return compareTo((IceInternal.EndpointI)obj) == 0;
        }
        catch(ClassCastException ee)
        {
            assert(false);
            return false;
        }
    }

    @Override
    public int
    compareTo(IceInternal.EndpointI obj) // From java.lang.Comparable
    {
        EndpointI p = null;

        try
        {
            p = (EndpointI)obj;
        }
        catch(ClassCastException ex)
        {
            return type() < obj.type() ? -1 : 1;
        }

        if(this == p)
        {
            return 0;
        }

        return _endpoint.compareTo(p._endpoint);
    }

    public IceInternal.EndpointI
    delegate()
    {
        return _endpoint;
    }

    private IceInternal.EndpointI _endpoint;
    private Configuration _configuration;
}
