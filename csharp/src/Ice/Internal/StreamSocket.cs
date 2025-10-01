// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Net;
using System.Net.Sockets;

namespace Ice.Internal;

#pragma warning disable CA1001 // _readEventArgs and _writeEventArgs are disposed by destroy.
internal sealed class StreamSocket
#pragma warning restore CA1001
{
    public StreamSocket(ProtocolInstance instance, NetworkProxy proxy, EndPoint addr, EndPoint sourceAddr)
    {
        _instance = instance;
        _proxy = proxy;
        _addr = addr;
        _sourceAddr = sourceAddr;
        _fd = Network.createSocket(false, (_proxy != null ? _proxy.getAddress() : _addr).AddressFamily);
        _state = StateNeedConnect;

        init();
    }

    public StreamSocket(ProtocolInstance instance, Socket fd)
    {
        _instance = instance;
        _fd = fd;
        _state = StateConnected;
        try
        {
            _desc = Network.fdToString(_fd);
        }
        catch (System.Exception)
        {
            Network.closeSocketNoThrow(_fd);
            throw;
        }
        init();
    }

    public int connect(Buffer readBuffer, Buffer writeBuffer, ref bool moreData)
    {
        if (_state == StateNeedConnect)
        {
            _state = StateConnectPending;
            return SocketOperation.Connect;
        }
        else if (_state <= StateConnectPending)
        {
            if (_writeEventArgs.SocketError != SocketError.Success)
            {
                var ex = new System.Net.Sockets.SocketException((int)_writeEventArgs.SocketError);
                if (Network.connectionRefused(ex))
                {
                    throw new Ice.ConnectionRefusedException(ex);
                }
                else
                {
                    throw new Ice.ConnectFailedException(ex);
                }
            }
            _desc = Network.fdToString(_fd, _proxy, _addr);
            _state = _proxy != null ? StateProxyWrite : StateConnected;
        }

        if (_state == StateProxyWrite)
        {
            _proxy.beginWrite(_addr, writeBuffer);
            return SocketOperation.Write;
        }
        else if (_state == StateProxyRead)
        {
            _proxy.beginRead(readBuffer);
            return SocketOperation.Read;
        }
        else if (_state == StateProxyConnected)
        {
            _proxy.finish(readBuffer, writeBuffer);

            readBuffer.clear();
            writeBuffer.clear();

            _state = StateConnected;
        }

        Debug.Assert(_state == StateConnected);
        return SocketOperation.None;
    }

    public Socket fd() => _fd;

    public void setBufferSize(int rcvSize, int sndSize) => Network.setTcpBufSize(_fd, rcvSize, sndSize, _instance);

    public int read(Buffer buf)
    {
        if (_state == StateProxyRead)
        {
            while (true)
            {
                int ret = read(buf.b);
                if (ret == 0)
                {
                    return SocketOperation.Read;
                }

                _state = toState(_proxy.endRead(buf));
                if (_state != StateProxyRead)
                {
                    return SocketOperation.None;
                }
            }
        }
        read(buf.b);
        return buf.b.hasRemaining() ? SocketOperation.Read : SocketOperation.None;
    }

    public int write(Buffer buf)
    {
        if (_state == StateProxyWrite)
        {
            while (true)
            {
                int ret = write(buf.b);
                if (ret == 0)
                {
                    return SocketOperation.Write;
                }
                _state = toState(_proxy.endWrite(buf));
                if (_state != StateProxyWrite)
                {
                    return SocketOperation.None;
                }
            }
        }
        write(buf.b);
        return buf.b.hasRemaining() ? SocketOperation.Write : SocketOperation.None;
    }

    public bool startRead(Buffer buf, AsyncCallback callback, object state)
    {
        Debug.Assert(_fd != null && _readEventArgs != null);

        try
        {
            _readCallback = callback;
            _readEventArgs.UserToken = state;
            _readEventArgs.SetBuffer(buf.b.rawBytes(), buf.b.position(), buf.b.remaining());
            return !_fd.ReceiveAsync(_readEventArgs);
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            if (Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            throw new Ice.SocketException(ex);
        }
    }

    public void finishRead(Buffer buf)
    {
        if (_fd == null) // Transceiver was closed
        {
            return;
        }

        Debug.Assert(_fd != null && _readEventArgs != null);
        try
        {
            if (_readEventArgs.SocketError != SocketError.Success)
            {
                throw new System.Net.Sockets.SocketException((int)_readEventArgs.SocketError);
            }
            int ret = _readEventArgs.BytesTransferred;
            _readEventArgs.SetBuffer(null, 0, 0);

            if (ret == 0)
            {
                throw new Ice.ConnectionLostException();
            }

            Debug.Assert(ret > 0);
            buf.b.position(buf.b.position() + ret);

            if (_state == StateProxyRead)
            {
                _state = toState(_proxy.endRead(buf));
            }
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            if (Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            throw new Ice.SocketException(ex);
        }
        catch (ObjectDisposedException ex)
        {
            throw new Ice.ConnectionLostException(ex);
        }
    }

    public bool startWrite(Buffer buf, AsyncCallback callback, object state, out bool messageFullyWritten)
    {
        Debug.Assert(_fd != null && _writeEventArgs != null);
        if (_state == StateConnectPending)
        {
            messageFullyWritten = false;
            _writeCallback = callback;
            try
            {
                EndPoint addr = _proxy != null ? _proxy.getAddress() : _addr;
                if (_sourceAddr != null)
                {
                    Network.doBind(_fd, _sourceAddr);
                }
                _writeEventArgs.RemoteEndPoint = addr;
                _writeEventArgs.UserToken = state;
                return !_fd.ConnectAsync(_writeEventArgs);
            }
            catch (System.Exception ex)
            {
                throw new Ice.SocketException(ex);
            }
        }

        try
        {
            _writeCallback = callback;
            _writeEventArgs.UserToken = state;
            _writeEventArgs.SetBuffer(buf.b.rawBytes(), buf.b.position(), buf.b.remaining());
            bool completedSynchronously = !_fd.SendAsync(_writeEventArgs);
            messageFullyWritten = true;
            return completedSynchronously;
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            if (Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }
            throw new Ice.SocketException(ex);
        }
        catch (ObjectDisposedException ex)
        {
            throw new Ice.ConnectionLostException(ex);
        }
    }

    public void finishWrite(Buffer buf)
    {
        if (_fd == null) // Transceiver was closed
        {
            buf.b.position(buf.b.limit()); // Assume all the data was sent for at-most-once semantics.
            return;
        }

        Debug.Assert(_fd != null && _writeEventArgs != null);

        if (_state < StateConnected && _state != StateProxyWrite)
        {
            return;
        }

        try
        {
            if (_writeEventArgs.SocketError != SocketError.Success)
            {
                throw new System.Net.Sockets.SocketException((int)_writeEventArgs.SocketError);
            }
            int ret = _writeEventArgs.BytesTransferred;
            _writeEventArgs.SetBuffer(null, 0, 0);
            if (ret == 0)
            {
                throw new Ice.ConnectionLostException();
            }

            Debug.Assert(ret > 0);
            buf.b.position(buf.b.position() + ret);

            if (_state == StateProxyWrite)
            {
                _state = toState(_proxy.endWrite(buf));
            }
        }
        catch (System.Net.Sockets.SocketException ex)
        {
            if (Network.connectionLost(ex))
            {
                throw new Ice.ConnectionLostException(ex);
            }

            throw new Ice.SocketException(ex);
        }
        catch (ObjectDisposedException ex)
        {
            throw new Ice.ConnectionLostException(ex);
        }
    }

    public void close()
    {
        Debug.Assert(_fd != null);
        try
        {
            Network.closeSocket(_fd);
        }
        finally
        {
            _fd = null;
        }
    }

    public void destroy()
    {
        Debug.Assert(_readEventArgs != null && _writeEventArgs != null);
        _readEventArgs.Dispose();
        _writeEventArgs.Dispose();
    }

    public override string ToString() => _desc;

    private int read(ByteBuffer buf)
    {
        Debug.Assert(_fd != null);
        int read = 0;
        while (buf.hasRemaining())
        {
            try
            {
                int ret = _fd.Receive(buf.rawBytes(), buf.position(), buf.remaining(), SocketFlags.None);
                if (ret == 0)
                {
                    throw new Ice.ConnectionLostException();
                }
                read += ret;
                buf.position(buf.position() + ret);
            }
            catch (System.Net.Sockets.SocketException ex)
            {
                if (Network.wouldBlock(ex))
                {
                    return read;
                }
                else if (Network.interrupted(ex))
                {
                    continue;
                }
                else if (Network.connectionLost(ex))
                {
                    throw new ConnectionLostException(ex);
                }

                throw new SocketException(ex);
            }
        }
        return read;
    }

    private int write(ByteBuffer buf)
    {
        Debug.Assert(_fd != null);
        int sent = 0;
        while (buf.hasRemaining())
        {
            try
            {
                int ret = _fd.Send(buf.rawBytes(), buf.position(), buf.remaining(), SocketFlags.None);
                Debug.Assert(ret > 0);

                sent += ret;
                buf.position(buf.position() + ret);
            }
            catch (System.Net.Sockets.SocketException ex)
            {
                if (Network.wouldBlock(ex))
                {
                    return sent;
                }
                else if (Network.connectionLost(ex))
                {
                    throw new Ice.ConnectionLostException(ex);
                }
                throw new Ice.SocketException(ex);
            }
        }
        return sent;
    }

    private void ioCompleted(object sender, SocketAsyncEventArgs e)
    {
        switch (e.LastOperation)
        {
            case SocketAsyncOperation.Receive:
                _readCallback(e.UserToken);
                break;
            case SocketAsyncOperation.Send:
            case SocketAsyncOperation.Connect:
                _writeCallback(e.UserToken);
                break;
            default:
                throw new ArgumentException("The last operation completed on the socket was not a receive or send");
        }
    }

    private void init()
    {
        Network.setBlock(_fd, false);
        Network.setTcpBufSize(_fd, _instance);

        _readEventArgs = new SocketAsyncEventArgs();
        _readEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);

        _writeEventArgs = new SocketAsyncEventArgs();
        _writeEventArgs.Completed += new EventHandler<SocketAsyncEventArgs>(ioCompleted);
    }

    private int toState(int operation)
    {
        return operation switch
        {
            SocketOperation.Read => StateProxyRead,
            SocketOperation.Write => StateProxyWrite,
            _ => StateProxyConnected,
        };
    }

    private readonly ProtocolInstance _instance;
    private readonly NetworkProxy _proxy;
    private readonly EndPoint _addr;
    private readonly EndPoint _sourceAddr;

    private Socket _fd;
    private int _state;
    private string _desc;

    private SocketAsyncEventArgs _writeEventArgs;
    private SocketAsyncEventArgs _readEventArgs;
    private AsyncCallback _writeCallback;
    private AsyncCallback _readCallback;

    private const int StateNeedConnect = 0;
    private const int StateConnectPending = 1;
    private const int StateProxyWrite = 2;
    private const int StateProxyRead = 3;
    private const int StateProxyConnected = 4;
    private const int StateConnected = 5;
}
