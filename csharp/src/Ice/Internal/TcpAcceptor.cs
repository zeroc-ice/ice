// Copyright (c) ZeroC, Inc.

using System.Net;
using System.Net.Sockets;
using System.Text;

namespace Ice.Internal;

internal class TcpAcceptor : Acceptor
{
    public virtual void close()
    {
        if (_acceptFd != null)
        {
            Network.closeSocketNoThrow(_acceptFd);
            _acceptFd = null;
        }
        if (_fd != null)
        {
            Network.closeSocketNoThrow(_fd);
            _fd = null;
        }
    }

    public virtual EndpointI listen()
    {
        try
        {
            _addr = Network.doBind(_fd, _addr);
            Network.doListen(_fd, _backlog);
        }
        catch (SystemException)
        {
            _fd = null;
            throw;
        }
        _endpoint = _endpoint.endpoint(this);
        return _endpoint;
    }

    public virtual bool startAccept(AsyncCallback callback, object state)
    {
        try
        {
            _result = _fd.BeginAccept(
                (IAsyncResult result) =>
                {
                    if (!result.CompletedSynchronously)
                    {
                        callback(result.AsyncState);
                    }
                },
                state);
        }
        catch (System.Exception ex)
        {
            throw new Ice.SocketException(ex);
        }
        return _result.CompletedSynchronously;
    }

    public virtual void finishAccept()
    {
        if (_fd != null)
        {
            _acceptFd = null;
            try
            {
                _acceptFd = _fd.EndAccept(_result);
            }
            catch (System.Exception ex)
            {
                _acceptError = new Ice.SocketException(ex);
            }
        }
    }

    public virtual Transceiver accept()
    {
        if (_acceptFd == null)
        {
            throw _acceptError;
        }

        Socket acceptFd = _acceptFd;
        _acceptFd = null;
        _acceptError = null;
        return new TcpTransceiver(_instance, new StreamSocket(_instance, acceptFd));
    }

    public string protocol()
    {
        return _instance.protocol();
    }

    public override string ToString()
    {
        return Network.addrToString(_addr);
    }

    public string toDetailedString() => "$local address = {this}";

    internal int effectivePort()
    {
        return _addr.Port;
    }

    internal TcpAcceptor(TcpEndpointI endpoint, ProtocolInstance instance, string host, int port)
    {
        _endpoint = endpoint;
        _instance = instance;
        _backlog = instance.properties().getIcePropertyAsInt("Ice.TCP.Backlog");

        try
        {
            int protocol = _instance.protocolSupport();
            _addr = (IPEndPoint)Network.getAddressForServer(host, port, protocol, _instance.preferIPv6());
            _fd = Network.createServerSocket(false, _addr.AddressFamily, protocol);
            Network.setBlock(_fd, false);
            Network.setTcpBufSize(_fd, _instance);
        }
        catch (System.Exception)
        {
            _fd = null;
            throw;
        }
    }

    private TcpEndpointI _endpoint;
    private readonly ProtocolInstance _instance;
    private Socket _fd;
    private Socket _acceptFd;
    private Ice.SocketException _acceptError;
    private int _backlog;
    private IPEndPoint _addr;
    private IAsyncResult _result;
}
