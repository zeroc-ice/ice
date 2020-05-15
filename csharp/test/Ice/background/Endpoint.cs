//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Test;

using ZeroC.Ice;

internal class Endpoint : ZeroC.Ice.Endpoint
{
    public override string ConnectionId => _endpoint.ConnectionId;
    public override bool HasCompressionFlag => _endpoint.HasCompressionFlag;
    public override bool IsDatagram => _endpoint.IsDatagram;

    public override bool IsSecure => _endpoint.IsSecure;
    public override string Name => _endpoint.Name;

    public override int Timeout => _endpoint.Timeout;
    public override EndpointType Type => (EndpointType)(TYPE_BASE + (short)_endpoint.Type);

    internal static short TYPE_BASE = 100;
    private ZeroC.Ice.Endpoint _endpoint;
    private Configuration _configuration;

    public override bool Equals(ZeroC.Ice.Endpoint? other)
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

    public override bool Equivalent(ZeroC.Ice.Endpoint endpoint)
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

    public override void IceWritePayload(OutputStream ostr)
    {
        ostr.WriteShort((short)_endpoint.Type);
        _endpoint.IceWritePayload(ostr);
    }

    public override ZeroC.Ice.Endpoint NewCompressionFlag(bool compressionFlag)
    {
        ZeroC.Ice.Endpoint endpoint = _endpoint.NewCompressionFlag(compressionFlag);
        if (endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new Endpoint(endpoint);
        }
    }

    public override ZeroC.Ice.Endpoint NewConnectionId(string connectionId)
    {
        ZeroC.Ice.Endpoint endpoint = _endpoint.NewConnectionId(connectionId);
        if (endpoint == _endpoint)
        {
            return this;
        }
        else
        {
            return new Endpoint(endpoint);
        }
    }

    public override ZeroC.Ice.Endpoint NewTimeout(int timeout)
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

    public override async ValueTask<IEnumerable<IceInternal.IConnector>>
        ConnectorsAsync(EndpointSelectionType selType)
    {
        _configuration.CheckConnectorsException();
        IEnumerable<IceInternal.IConnector> connectors = await _endpoint.ConnectorsAsync(selType).ConfigureAwait(false);
        return connectors.Select(item => new Connector(item));
    }

    public override IEnumerable<ZeroC.Ice.Endpoint> ExpandHost(out ZeroC.Ice.Endpoint? publish)
    {
        var endpoints = _endpoint.ExpandHost(out publish).Select(endpoint => GetEndpoint(endpoint));
        if (publish != null)
        {
            publish = GetEndpoint(publish);
        }
        return endpoints;
    }

    public override IEnumerable<ZeroC.Ice.Endpoint> ExpandIfWildcard() =>
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

    internal Endpoint(ZeroC.Ice.Endpoint endpoint)
    {
        _endpoint = endpoint;
        _configuration = Configuration.GetInstance();
    }

    internal Endpoint GetEndpoint(ZeroC.Ice.Endpoint del) => del == _endpoint ? this : new Endpoint(del);
}
