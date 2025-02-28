// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Net.Security;

internal class EndpointI : Ice.Internal.EndpointI
{
    internal static short TYPE_BASE = 100;

    internal EndpointI(Ice.Internal.EndpointI endpoint)
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

    public override Ice.Internal.EndpointI timeout(int timeout)
    {
        Ice.Internal.EndpointI endpoint = _endpoint.timeout(timeout);
        if (endpoint == _endpoint)
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

    public override Ice.Internal.EndpointI connectionId(string connectionId)
    {
        Ice.Internal.EndpointI endpoint = _endpoint.connectionId(connectionId);
        if (endpoint == _endpoint)
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

    public override Ice.Internal.EndpointI compress(bool compress)
    {
        Ice.Internal.EndpointI endpoint = _endpoint.compress(compress);
        if (endpoint == _endpoint)
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

    public override Ice.Internal.Transceiver transceiver()
    {
        Ice.Internal.Transceiver transceiver = _endpoint.transceiver();
        if (transceiver != null)
        {
            return new Transceiver(transceiver);
        }
        else
        {
            return null;
        }
    }

    private class ConnectorsCallback : Ice.Internal.EndpointI_connectors
    {
        internal ConnectorsCallback(Ice.Internal.EndpointI_connectors cb)
        {
            _callback = cb;
        }

        public void connectors(List<Ice.Internal.Connector> cons)
        {
            List<Ice.Internal.Connector> connectors = new List<Ice.Internal.Connector>();
            foreach (Ice.Internal.Connector connector in cons)
            {
                connectors.Add(new Connector(connector));
            }
            _callback.connectors(connectors);
        }

        public void exception(Ice.LocalException exception)
        {
            _callback.exception(exception);
        }

        private Ice.Internal.EndpointI_connectors _callback;
    }

    public override void connectors_async(Ice.Internal.EndpointI_connectors cb)
    {
        try
        {
            _configuration.checkConnectorsException();
            _endpoint.connectors_async(new ConnectorsCallback(cb));
        }
        catch (Ice.LocalException ex)
        {
            cb.exception(ex);
        }
    }

    public override Ice.Internal.Acceptor acceptor(
        string adapterName,
        SslServerAuthenticationOptions serverAuthenticationOptions)
    {
        return new Acceptor(this, _endpoint.acceptor(adapterName, serverAuthenticationOptions));
    }

    public EndpointI endpoint(Ice.Internal.EndpointI delEndp)
    {
        if (delEndp == _endpoint)
        {
            return this;
        }
        else
        {
            return new EndpointI(delEndp);
        }
    }

    public override List<Ice.Internal.EndpointI> expandHost() =>
        _endpoint.expandHost().Select(e => new EndpointI(e) as Ice.Internal.EndpointI).ToList();

    public override bool isLoopbackOrMulticast() => _endpoint.isLoopbackOrMulticast();

    public override EndpointI toPublishedEndpoint(string publishedHost) =>
        endpoint(_endpoint.toPublishedEndpoint(publishedHost));

    public override bool equivalent(Ice.Internal.EndpointI endpoint)
    {
        EndpointI testEndpoint = null;
        try
        {
            testEndpoint = (EndpointI)endpoint;
        }
        catch (System.InvalidCastException)
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

    public override int CompareTo(Ice.Internal.EndpointI obj)
    {
        EndpointI p = null;

        try
        {
            p = (EndpointI)obj;
        }
        catch (System.InvalidCastException)
        {
            try
            {
                return type() < obj.type() ? -1 : 1;
            }
            catch (System.InvalidCastException)
            {
                Debug.Assert(false);
            }
        }

        if (this == p)
        {
            return 0;
        }

        return _endpoint.CompareTo(p._endpoint);
    }

    public Ice.Internal.EndpointI getDelegate()
    {
        return _endpoint;
    }

    private Ice.Internal.EndpointI _endpoint;
    private Configuration _configuration;
}
