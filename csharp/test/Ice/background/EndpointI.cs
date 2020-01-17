//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

    public override Ice.EndpointInfo getInfo()
    {
        return _endpoint.getInfo();
    }

    public override void streamWriteImpl(Ice.OutputStream s)
    {
        s.WriteShort(_endpoint.type());
        _endpoint.streamWriteImpl(s);
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

    public override IceInternal.Endpoint timeout(int timeout)
    {
        var endpoint = _endpoint.timeout(timeout);
        if (endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new Endpoint(endpoint);
        }
    }

    public override string connectionId()
    {
        return _endpoint.connectionId();
    }

    public override IceInternal.Endpoint connectionId(string connectionId)
    {
        IceInternal.Endpoint endpoint = _endpoint.connectionId(connectionId);
        if (endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new Endpoint(endpoint);
        }
    }

    public override bool compress()
    {
        return _endpoint.compress();
    }

    public override IceInternal.Endpoint compress(bool compress)
    {
        IceInternal.Endpoint endpoint = _endpoint.compress(compress);
        if (endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new Endpoint(endpoint);
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

    public override IceInternal.ITransceiver? transceiver()
    {
        IceInternal.ITransceiver? transceiver = _endpoint.transceiver();
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

        public void connectors(List<IceInternal.IConnector> cons)
        {
            var connectors = new List<IceInternal.IConnector>();
            foreach (IceInternal.IConnector connector in cons)
            {
                connectors.Add(new Connector(connector));
            }
            _callback.connectors(connectors);
        }

        public void exception(Ice.LocalException exception)
        {
            _callback.exception(exception);
        }

        private IceInternal.IEndpointConnectors _callback;
    }

    public override void connectors_async(Ice.EndpointSelectionType selType, IceInternal.IEndpointConnectors cb)
    {
        try
        {
            _configuration.checkConnectorsException();
            _endpoint.connectors_async(selType, new ConnectorsCallback(cb));
        }
        catch (Ice.LocalException ex)
        {
            cb.exception(ex);
        }
    }

    public override IceInternal.IAcceptor acceptor(string adapterName)
    {
        var acceptor = _endpoint.acceptor(adapterName);
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

    public override List<IceInternal.Endpoint> expandIfWildcard()
    {
        var endps = new List<IceInternal.Endpoint>();
        foreach (var endpt in _endpoint.expandIfWildcard())
        {
            endps.Add(endpt == _endpoint ? this : new Endpoint(endpt));
        }
        return endps;
    }

    public override List<IceInternal.Endpoint> expandHost(out IceInternal.Endpoint? publish)
    {
        var endps = new List<IceInternal.Endpoint>();
        foreach (var endpt in _endpoint.expandHost(out publish))
        {
            endps.Add(endpt == _endpoint ? this : new Endpoint(endpt));
        }
        if (publish != null)
        {
            publish = publish == _endpoint ? this : new Endpoint(publish);
        }
        return endps;
    }

    public override bool equivalent(IceInternal.Endpoint endpoint)
    {
        if (!(endpoint is Endpoint))
        {
            return false;
        }
        return ((Endpoint)endpoint)._endpoint.equivalent(_endpoint);
    }

    public override string options()
    {
        return _endpoint.options();
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
            return type() < obj.type() ? -1 : 1;
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
