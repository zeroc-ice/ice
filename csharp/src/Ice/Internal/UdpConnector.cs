// Copyright (c) ZeroC, Inc.

using System.Net;

namespace Ice.Internal;

internal sealed class UdpConnector : Connector
{
    public Transceiver connect()
    {
        return new UdpTransceiver(_instance, _addr, _sourceAddr, _mcastInterface, _mcastTtl);
    }

    public short type()
    {
        return _instance.type();
    }

    //
    // Only for use by UdpEndpointI
    //
    internal UdpConnector(
        ProtocolInstance instance,
        EndPoint addr,
        EndPoint sourceAddr,
        string mcastInterface,
        int mcastTtl,
        string connectionId)
    {
        _instance = instance;
        _addr = addr;
        _sourceAddr = sourceAddr;
        _mcastInterface = mcastInterface;
        _mcastTtl = mcastTtl;
        _connectionId = connectionId;
    }

    public override bool Equals(object obj)
    {
        if (!(obj is UdpConnector))
        {
            return false;
        }

        if (this == obj)
        {
            return true;
        }

        UdpConnector p = (UdpConnector)obj;
        if (!_connectionId.Equals(p._connectionId, StringComparison.Ordinal))
        {
            return false;
        }

        if (!_mcastInterface.Equals(p._mcastInterface, StringComparison.Ordinal))
        {
            return false;
        }

        if (_mcastTtl != p._mcastTtl)
        {
            return false;
        }

        if (!Network.addressEquals(_sourceAddr, p._sourceAddr))
        {
            return false;
        }

        return _addr.Equals(p._addr);
    }

    public override string ToString()
    {
        return Network.addrToString(_addr);
    }

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(_addr);
        if (_sourceAddr is not null)
        {
            hash.Add(_sourceAddr);
        }
        hash.Add(_mcastInterface);
        hash.Add(_mcastTtl);
        hash.Add(_connectionId);
        return hash.ToHashCode();
    }

    private readonly ProtocolInstance _instance;
    private readonly EndPoint _addr;
    private readonly EndPoint _sourceAddr;
    private readonly string _mcastInterface;
    private readonly int _mcastTtl;
    private readonly string _connectionId;
}
