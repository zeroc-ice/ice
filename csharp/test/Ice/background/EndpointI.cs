//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;

using System.Collections.Generic;
using System.Diagnostics;

internal class Endpoint : IceInternal.Endpoint
{
    internal static short TYPE_BASE = 100;

    internal Endpoint(IceInternal.Endpoint endpoint)
    {
        _endpoint = endpoint;
        _configuration = Configuration.getInstance();
    }

    public override string ToString()
    {
        return "test-" + _endpoint.ToString();
    }

    public override Ice.EndpointInfo GetInfo()
    {
        return _endpoint.GetInfo();
    }

    public override void StreamWriteImpl(Ice.OutputStream s)
    {
        s.WriteShort((short)_endpoint.Type());
        _endpoint.StreamWriteImpl(s);
    }

    public override EndpointType Type()
    {
        return (EndpointType)(TYPE_BASE + (short)_endpoint.Type());
    }

    public override string Transport()
    {
        return _endpoint.Transport();
    }

    public override int Timeout()
    {
        return _endpoint.Timeout();
    }

    public override IceInternal.Endpoint Timeout(int timeout)
    {
        var endpoint = _endpoint.Timeout(timeout);
        if (endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new Endpoint(endpoint);
        }
    }

    public override string ConnectionId()
    {
        return _endpoint.ConnectionId();
    }

    public override IceInternal.Endpoint ConnectionId(string connectionId)
    {
        IceInternal.Endpoint endpoint = _endpoint.ConnectionId(connectionId);
        if (endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new Endpoint(endpoint);
        }
    }

    public override bool Compress()
    {
        return _endpoint.Compress();
    }

    public override IceInternal.Endpoint Compress(bool compress)
    {
        IceInternal.Endpoint endpoint = _endpoint.Compress(compress);
        if (endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new Endpoint(endpoint);
        }
    }

    public override bool Datagram()
    {
        return _endpoint.Datagram();
    }

    public override bool Secure()
    {
        return _endpoint.Secure();
    }

    public override IceInternal.ITransceiver? Transceiver()
    {
        IceInternal.ITransceiver? transceiver = _endpoint.Transceiver();
        if (transceiver != null)
        {
            return new Transceiver(transceiver);
        }
        else
        {
            return null;
        }
    }

    private class ConnectorsCallback : IceInternal.IEndpointConnectors
    {
        internal ConnectorsCallback(IceInternal.IEndpointConnectors cb)
        {
            _callback = cb;
        }

        public void Connectors(List<IceInternal.IConnector> cons)
        {
            var connectors = new List<IceInternal.IConnector>();
            foreach (IceInternal.IConnector connector in cons)
            {
                connectors.Add(new Connector(connector));
            }
            _callback.Connectors(connectors);
        }

        public void Exception(System.Exception exception)
        {
            _callback.Exception(exception);
        }

        private IceInternal.IEndpointConnectors _callback;
    }

    public override void ConnectorsAsync(Ice.EndpointSelectionType selType, IceInternal.IEndpointConnectors cb)
    {
        try
        {
            _configuration.checkConnectorsException();
            _endpoint.ConnectorsAsync(selType, new ConnectorsCallback(cb));
        }
        catch (System.Exception ex)
        {
            cb.Exception(ex);
        }
    }

    public override IceInternal.IAcceptor Acceptor(string adapterName)
    {
        var acceptor = _endpoint.Acceptor(adapterName);
        Debug.Assert(acceptor != null);
        return new Acceptor(this, acceptor);
    }

    public Endpoint endpoint(IceInternal.Endpoint delEndp)
    {
        if (delEndp == _endpoint)
        {
            return this;
        }
        else
        {
            return new Endpoint(delEndp);
        }
    }

    public override List<IceInternal.Endpoint> ExpandIfWildcard()
    {
        var endps = new List<IceInternal.Endpoint>();
        foreach (var endpt in _endpoint.ExpandIfWildcard())
        {
            endps.Add(endpt == _endpoint ? this : new Endpoint(endpt));
        }
        return endps;
    }

    public override List<IceInternal.Endpoint> ExpandHost(out IceInternal.Endpoint? publish)
    {
        var endps = new List<IceInternal.Endpoint>();
        foreach (var endpt in _endpoint.ExpandHost(out publish))
        {
            endps.Add(endpt == _endpoint ? this : new Endpoint(endpt));
        }
        if (publish != null)
        {
            publish = publish == _endpoint ? this : new Endpoint(publish);
        }
        return endps;
    }

    public override bool Equivalent(IceInternal.Endpoint endpoint)
    {
        if (!(endpoint is Endpoint))
        {
            return false;
        }
        return ((Endpoint)endpoint)._endpoint.Equivalent(_endpoint);
    }

    public override string Options()
    {
        return _endpoint.Options();
    }

    public override int GetHashCode()
    {
        return _endpoint.GetHashCode();
    }

    public override int CompareTo(IceInternal.Endpoint obj)
    {
        if (ReferenceEquals(this, obj))
        {
            return 0;
        }

        if (!(obj is Endpoint))
        {
            return Type() < obj.Type() ? -1 : 1;
        }

        return _endpoint.CompareTo(((Endpoint)obj)._endpoint);
    }

    public IceInternal.Endpoint getDelegate()
    {
        return _endpoint;
    }

    private IceInternal.Endpoint _endpoint;
    private Configuration _configuration;
}
