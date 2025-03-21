// Copyright (c) ZeroC, Inc.

using System.Net;

namespace Ice.Internal;

internal sealed class TcpConnector : Connector
{
    public Transceiver connect()
    {
        return new TcpTransceiver(_instance, new StreamSocket(_instance, _proxy, _addr, _sourceAddr));
    }

    public short type()
    {
        return _instance.type();
    }

    //
    // Only for use by TcpEndpoint
    //
    internal TcpConnector(
        ProtocolInstance instance,
        EndPoint addr,
        NetworkProxy proxy,
        EndPoint sourceAddr,
        int timeout,
        string connectionId)
    {
        _instance = instance;
        _addr = addr;
        _proxy = proxy;
        _sourceAddr = sourceAddr;
        _timeout = timeout;
        _connectionId = connectionId;
    }

    public override bool Equals(object obj)
    {
        if (!(obj is TcpConnector))
        {
            return false;
        }

        if (this == obj)
        {
            return true;
        }

        TcpConnector p = (TcpConnector)obj;
        if (_timeout != p._timeout)
        {
            return false;
        }

        if (!Network.addressEquals(_sourceAddr, p._sourceAddr))
        {
            return false;
        }

        if (!_connectionId.Equals(p._connectionId, StringComparison.Ordinal))
        {
            return false;
        }

        return _addr.Equals(p._addr);
    }

    public override string ToString()
    {
        return Network.addrToString(_proxy == null ? _addr : _proxy.getAddress());
    }

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(_addr);
        if (_sourceAddr is not null)
        {
            hash.Add(_sourceAddr);
        }
        hash.Add(_timeout);
        hash.Add(_connectionId);
        return hash.ToHashCode();
    }

    private readonly ProtocolInstance _instance;
    private readonly EndPoint _addr;
    private readonly NetworkProxy _proxy;
    private readonly EndPoint _sourceAddr;
    private readonly int _timeout;
    private readonly string _connectionId;
}
