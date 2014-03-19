// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

    //
    // Marshal the endpoint
    //
    public override void streamWrite(IceInternal.BasicStream s)
    {
        s.writeShort(type());
        _endpoint.streamWrite(s);
    }

    //
    // Convert the endpoint to its string form
    //
    public override string ice_toString_()
    {
        return "test-" + _endpoint.ToString();
    }

    public override Ice.EndpointInfo getInfo()
    {
        return _endpoint.getInfo();
    }

    //
    // Return the endpoint type
    //
    public override short type()
    {
        return (short)(TYPE_BASE + _endpoint.type());
    }

    //
    // Return the protocol name;
    //
    public override string protocol()
    {
        return _endpoint.protocol();
    }

    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    public override int timeout()
    {
        return _endpoint.timeout();
    }

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
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

    //
    // Return a new endpoint with a different connection id.
    //
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

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    public override bool compress()
    {
        return _endpoint.compress();
    }

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
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

    //
    // Return true if the endpoint is datagram-based.
    //
    public override bool datagram()
    {
        return _endpoint.datagram();
    }

    //
    // Return true if the endpoint is secure.
    //
    public override bool secure()
    {
        return _endpoint.secure();
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public override IceInternal.Transceiver transceiver(ref IceInternal.EndpointI endpoint)
    {
        IceInternal.Transceiver transceiver = _endpoint.transceiver(ref endpoint);
        if(endpoint == _endpoint)
        {
            endpoint = this;
        }
        else
        {
            endpoint = new EndpointI(endpoint);
        }

        if(transceiver != null)
        {
            return new Transceiver(transceiver);
        }
        else
        {
            return null;
        }
    }

    //
    // Return connectors for this endpoint, or empty list if no connector
    // is available.
    //
    public override List<IceInternal.Connector> connectors(Ice.EndpointSelectionType selType)
    {
        _configuration.checkConnectorsException();
        List<IceInternal.Connector> connectors = new List<IceInternal.Connector>();
        foreach(IceInternal.Connector connector in _endpoint.connectors(selType))
        {
            connectors.Add(new Connector(connector));
        }
        return connectors;
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

    public override IceInternal.Acceptor acceptor(ref IceInternal.EndpointI endpoint, string adapterName)
    {
        Acceptor p = new Acceptor(_endpoint.acceptor(ref endpoint, adapterName));
        endpoint = new EndpointI(endpoint);
        return p;
    }

    public override List<IceInternal.EndpointI> expand()
    {
        List<IceInternal.EndpointI> endps = new List<IceInternal.EndpointI>();
        foreach(IceInternal.EndpointI endpt in _endpoint.expand())
        {
            endps.Add(endpt == _endpoint ? this : new EndpointI(endpt));
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

    public override int GetHashCode()
    {
        return _endpoint.GetHashCode();
    }

    //
    // Compare endpoints for sorting purposes
    //
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

    private IceInternal.EndpointI _endpoint;
    private Configuration _configuration;
}
