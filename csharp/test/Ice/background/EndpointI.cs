// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using System.Diagnostics;

internal class EndpointI : IceInternal.EndpointI
{
    internal static short TYPE_BASE = 100;

    internal EndpointI(IceInternal.EndpointI endpoint)
    {
        _endpoint = endpoint;
        _configuration = Configuration.getInstance();
    }

    public override string ice_toString_()
    {
        return "test-" + _endpoint.ToString();
    }

    public override Ice.EndpointInfo getInfo()
    {
        return _endpoint.getInfo();
    }

    public override void streamWriteImpl(Ice.OutputStream s)
    {
        s.writeShort(_endpoint.type());
        _endpoint.streamWrite(s);
    }

    public override short type()
    {
        return (short)(TYPE_BASE + _endpoint.type());
    }

    public override string protocol()
    {
        return _endpoint.protocol();
    }

    public override int timeout()
    {
        return _endpoint.timeout();
    }

    public override IceInternal.EndpointI timeout(int timeout)
    {
        IceInternal.EndpointI endpoint = _endpoint.timeout(timeout);
        if(endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new EndpointI(endpoint);
        }
    }

    public override string connectionId()
    {
        return _endpoint.connectionId();
    }

    public override IceInternal.EndpointI connectionId(string connectionId)
    {
        IceInternal.EndpointI endpoint = _endpoint.connectionId(connectionId);
        if(endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new EndpointI(endpoint);
        }
    }

    public override bool compress()
    {
        return _endpoint.compress();
    }

    public override IceInternal.EndpointI compress(bool compress)
    {
        IceInternal.EndpointI endpoint = _endpoint.compress(compress);
        if(endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new EndpointI(endpoint);
        }
    }

    public override bool datagram()
    {
        return _endpoint.datagram();
    }

    public override bool secure()
    {
        return _endpoint.secure();
    }

    public override IceInternal.Transceiver transceiver()
    {
        IceInternal.Transceiver transceiver = _endpoint.transceiver();
        if(transceiver != null)
        {
            return new Transceiver(transceiver);
        }
        else
        {
            return null;
        }
    }

    private class ConnectorsCallback : IceInternal.EndpointI_connectors
    {
        internal ConnectorsCallback(IceInternal.EndpointI_connectors cb)
        {
            _callback = cb;
        }

        public void connectors(List<IceInternal.Connector> cons)
        {
            List<IceInternal.Connector> connectors = new List<IceInternal.Connector>();
            foreach(IceInternal.Connector connector in cons)
            {
                connectors.Add(new Connector(connector));
            }
            _callback.connectors(connectors);
        }

        public void exception(Ice.LocalException exception)
        {
            _callback.exception(exception);
        }

        private IceInternal.EndpointI_connectors _callback;
    }

    public override void connectors_async(Ice.EndpointSelectionType selType, IceInternal.EndpointI_connectors cb)
    {
        try
        {
            _configuration.checkConnectorsException();
            _endpoint.connectors_async(selType, new ConnectorsCallback(cb));
        }
        catch(Ice.LocalException ex)
        {
            cb.exception(ex);
        }
    }

    public override IceInternal.Acceptor acceptor(string adapterName)
    {
        return new Acceptor(this, _endpoint.acceptor(adapterName));
    }

    public EndpointI endpoint(IceInternal.EndpointI delEndp)
    {
        if(delEndp == _endpoint)
        {
            return this;
        }
        else
        {
            return new EndpointI(delEndp);
        }
    }

    public override List<IceInternal.EndpointI> expandIfWildcard()
    {
        List<IceInternal.EndpointI> endps = new List<IceInternal.EndpointI>();
        foreach(IceInternal.EndpointI endpt in _endpoint.expandIfWildcard())
        {
            endps.Add(endpt == _endpoint ? this : new EndpointI(endpt));
        }
        return endps;
    }

    public override List<IceInternal.EndpointI> expandHost(out IceInternal.EndpointI publish)
    {
        List<IceInternal.EndpointI> endps = new List<IceInternal.EndpointI>();
        foreach(IceInternal.EndpointI endpt in _endpoint.expandHost(out publish))
        {
            endps.Add(endpt == _endpoint ? this : new EndpointI(endpt));
        }
        if(publish != null)
        {
            publish = publish == _endpoint ? this : new EndpointI(publish);
        }
        return endps;
    }

    public override bool equivalent(IceInternal.EndpointI endpoint)
    {
        EndpointI testEndpoint = null;
        try
        {
            testEndpoint = (EndpointI)endpoint;
        }
        catch(System.InvalidCastException)
        {
            return false;
        }
        return testEndpoint._endpoint.equivalent(_endpoint);
    }

    public override string options()
    {
        return _endpoint.options();
    }

    public override int GetHashCode()
    {
        return _endpoint.GetHashCode();
    }

    public override int CompareTo(IceInternal.EndpointI obj)
    {
        EndpointI p = null;

        try
        {
            p = (EndpointI)obj;
        }
        catch(System.InvalidCastException)
        {
            try
            {
                return type() < obj.type() ? -1 : 1;
            }
            catch(System.InvalidCastException)
            {
                Debug.Assert(false);
            }
        }

        if(this == p)
        {
            return 0;
        }

        return _endpoint.CompareTo(p._endpoint);
    }

    public IceInternal.EndpointI getDelegate()
    {
        return _endpoint;
    }

    private IceInternal.EndpointI _endpoint;
    private Configuration _configuration;
}
