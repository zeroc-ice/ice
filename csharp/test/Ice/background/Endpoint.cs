//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;
using System.Diagnostics;
using System.Linq;

internal class Endpoint : Ice.Endpoint
{
    public override string ConnectionId => _endpoint.ConnectionId;
    public override bool HasCompressionFlag => _endpoint.HasCompressionFlag;
    public override bool IsDatagram => _endpoint.IsDatagram;

    public override bool IsSecure => _endpoint.IsSecure;
    public override string Name => _endpoint.Name;

    public override int Timeout => _endpoint.Timeout;
    public override Ice.EndpointType Type => (Ice.EndpointType)(TYPE_BASE + (short)_endpoint.Type);

    internal static short TYPE_BASE = 100;
    private Ice.Endpoint _endpoint;
    private Configuration _configuration;

    public override bool Equals(Ice.Endpoint? other)
    {
        if (ReferenceEquals(this, other))
        {
            return true;
        }
        if (other is Endpoint testEndpoint)
        {
            return _endpoint.Equals(testEndpoint._endpoint);
        }
        else
        {
            return false;
        }
    }

    public override int GetHashCode() => _endpoint.GetHashCode();

    public override string ToString() => $"test-{_endpoint}";

    public override string OptionsToString() => _endpoint.OptionsToString();

    public override bool Equivalent(Ice.Endpoint endpoint)
    {
        if (endpoint is Endpoint testEndpoint)
        {
            return testEndpoint._endpoint.Equivalent(_endpoint);
        }
        else
        {
            return false;
        }
    }

    public override void IceWritePayload(Ice.OutputStream ostr)
    {
        ostr.WriteShort((short)_endpoint.Type);
        _endpoint.IceWritePayload(ostr);
    }

    public override Ice.Endpoint NewCompressionFlag(bool compressionFlag)
    {
        Ice.Endpoint endpoint = _endpoint.NewCompressionFlag(compressionFlag);
        if (endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new Endpoint(endpoint);
        }
    }

    public override Ice.Endpoint NewConnectionId(string connectionId)
    {
        Ice.Endpoint endpoint = _endpoint.NewConnectionId(connectionId);
        if (endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new Endpoint(endpoint);
        }
    }

    public override Ice.Endpoint NewTimeout(int timeout)
    {
        var endpoint = _endpoint.NewTimeout(timeout);
        if (endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new Endpoint(endpoint);
        }
    }

    public override IceInternal.IAcceptor GetAcceptor(string adapterName)
    {
        var acceptor = _endpoint.GetAcceptor(adapterName);
        TestHelper.Assert(acceptor != null);
        return new Acceptor(this, acceptor);
    }

    public override void ConnectorsAsync(Ice.EndpointSelectionType selType, Ice.IEndpointConnectors cb)
    {
        try
        {
            _configuration.CheckConnectorsException();
            _endpoint.ConnectorsAsync(selType, new ConnectorsCallback(cb));
        }
        catch (System.Exception ex)
        {
            cb.Exception(ex);
        }
    }

    public override IEnumerable<Ice.Endpoint> ExpandHost(out Ice.Endpoint? publish)
    {
        var endpoints = _endpoint.ExpandHost(out publish).Select(endpoint => GetEndpoint(endpoint));
        if (publish != null)
        {
            publish = GetEndpoint(publish);
        }
        return endpoints;
    }

    public override IEnumerable<Ice.Endpoint> ExpandIfWildcard() =>
        _endpoint.ExpandIfWildcard().Select(endpoint => GetEndpoint(endpoint));

    public override IceInternal.ITransceiver? GetTransceiver()
    {
        IceInternal.ITransceiver? transceiver = _endpoint.GetTransceiver();
        if (transceiver != null)
        {
            return new Transceiver(transceiver);
        }
        else
        {
            return null;
        }
    }

    internal Endpoint(Ice.Endpoint endpoint)
    {
        _endpoint = endpoint;
        _configuration = Configuration.GetInstance();
    }

    internal Endpoint GetEndpoint(Ice.Endpoint del) => del == _endpoint ? this : new Endpoint(del);

    private class ConnectorsCallback : Ice.IEndpointConnectors
    {
        private Ice.IEndpointConnectors _callback;

        internal ConnectorsCallback(Ice.IEndpointConnectors cb)
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
    }
}
