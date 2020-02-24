//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Net.Sockets;

internal class Transceiver : IceInternal.ITransceiver
{
    public Socket? Fd()
    {
        return _transceiver.Fd();
    }

    public int Initialize(IceInternal.Buffer readBuffer, IceInternal.Buffer writeBuffer, ref bool hasMoreData)
    {
        _configuration.checkInitializeException();
        if (!_initialized)
        {
            int status = _transceiver.Initialize(readBuffer, writeBuffer, ref hasMoreData);
            if (status != IceInternal.SocketOperation.None)
            {
                return status;
            }
            _initialized = true;
        }
        return IceInternal.SocketOperation.None;
    }

    public int Closing(bool initiator, Ice.LocalException? ex)
    {
        return _transceiver.Closing(initiator, ex);
    }

    public void Close()
    {
        _transceiver.Close();
    }

    public IceInternal.Endpoint Bind()
    {
        return _transceiver.Bind();
    }

    public int Write(IceInternal.Buffer buf)
    {
        if (!_configuration.writeReady() && buf.B.HasRemaining())
        {
            return IceInternal.SocketOperation.Write;
        }

        _configuration.checkWriteException();
        return _transceiver.Write(buf);
    }

    public int Read(IceInternal.Buffer buf, ref bool hasMoreData)
    {
        if (!_configuration.readReady() && buf.B.HasRemaining())
        {
            return IceInternal.SocketOperation.Read;
        }

        _configuration.checkReadException();

        if (_buffered)
        {
            while (buf.B.HasRemaining())
            {
                if (_readBufferPos == _readBuffer.B.Position())
                {
                    _readBufferPos = 0;
                    _readBuffer.B.Position(0);
                    _transceiver.Read(_readBuffer, ref hasMoreData);
                    if (_readBufferPos == _readBuffer.B.Position())
                    {
                        hasMoreData = false;
                        return IceInternal.SocketOperation.Read;
                    }
                }

                int pos = _readBuffer.B.Position();
                Debug.Assert(pos > _readBufferPos);
                int requested = buf.B.Remaining();
                int available = pos - _readBufferPos;
                Debug.Assert(available > 0);
                if (available >= requested)
                {
                    available = requested;
                }

                byte[] arr = new byte[available];
                _readBuffer.B.Position(_readBufferPos);
                _readBuffer.B.Get(arr);
                buf.B.Put(arr);
                _readBufferPos += available;
                _readBuffer.B.Position(pos);
            }
            hasMoreData = _readBufferPos < _readBuffer.B.Position();
            return IceInternal.SocketOperation.None;
        }
        else
        {
            return _transceiver.Read(buf, ref hasMoreData);
        }
    }

    public bool StartRead(IceInternal.Buffer buf, IceInternal.AsyncCallback callback, object state)
    {
        if (_configuration.readReady())
        {
            _configuration.checkReadException(); // Only raise if we're configured to read now.
        }
        if (_buffered)
        {
            int pos = _readBuffer.B.Position();
            int available = pos - _readBufferPos;
            if (available > 0)
            {
                int requested = buf.B.Remaining();
                if (available >= requested)
                {
                    available = requested;
                }

                byte[] arr = new byte[available];
                _readBuffer.B.Position(_readBufferPos);
                _readBuffer.B.Get(arr);
                buf.B.Put(arr);
                _readBufferPos += available;
                _readBuffer.B.Position(pos);
            }

            if (_readBufferPos == _readBuffer.B.Position() && buf.B.HasRemaining())
            {
                _readBufferPos = 0;
                _readBuffer.B.Position(0);
                return _transceiver.StartRead(_readBuffer, callback, state);
            }
            else
            {
                Debug.Assert(!buf.B.HasRemaining());
                return true; // Completed synchronously
            }
        }
        else
        {
            return _transceiver.StartRead(buf, callback, state);
        }
    }

    public void FinishRead(IceInternal.Buffer buf)
    {
        _configuration.checkReadException();
        if (_buffered)
        {
            if (buf.B.HasRemaining())
            {
                _transceiver.FinishRead(_readBuffer);

                int pos = _readBuffer.B.Position();
                int requested = buf.B.Remaining();
                int available = pos - _readBufferPos;
                if (available > 0)
                {
                    if (available >= requested)
                    {
                        available = requested;
                    }

                    byte[] arr = new byte[available];
                    _readBuffer.B.Position(_readBufferPos);
                    _readBuffer.B.Get(arr);
                    buf.B.Put(arr);
                    _readBufferPos += available;
                    _readBuffer.B.Position(pos);
                }
            }
        }
        else
        {
            _transceiver.FinishRead(buf);
        }
    }

    public bool StartWrite(IceInternal.Buffer buf, IceInternal.AsyncCallback callback, object state, out bool completed)
    {
        _configuration.checkWriteException();
        return _transceiver.StartWrite(buf, callback, state, out completed);
    }

    public void FinishWrite(IceInternal.Buffer buf)
    {
        _configuration.checkWriteException();
        _transceiver.FinishWrite(buf);
    }

    public string Transport()
    {
        return "test-" + _transceiver.Transport();
    }

    public Ice.ConnectionInfo GetInfo()
    {
        return _transceiver.GetInfo();
    }

    public override string? ToString()
    {
        return _transceiver.ToString();
    }

    public string ToDetailedString()
    {
        return _transceiver.ToDetailedString();
    }

    public void CheckSendSize(IceInternal.Buffer buf)
    {
        _transceiver.CheckSendSize(buf);
    }

    public void SetBufferSize(int rcvSize, int sndSize)
    {
        _transceiver.SetBufferSize(rcvSize, sndSize);
    }

    public void Destroy()
    {
        _transceiver.Destroy();
    }

    public IceInternal.ITransceiver GetDelegate()
    {
        return _transceiver;
    }

    //
    // Only for use by Connector, Acceptor
    //
    internal Transceiver(IceInternal.ITransceiver transceiver)
    {
        _transceiver = transceiver;
        _configuration = Configuration.getInstance();
        _initialized = false;
        _readBuffer = new IceInternal.Buffer();
        _readBuffer.Resize(1024 * 8, true); // 8KB buffer
        _readBuffer.B.Position(0);
        _readBufferPos = 0;
        _buffered = _configuration.buffered();
    }

    private IceInternal.ITransceiver _transceiver;
    private Configuration _configuration;
    private bool _initialized;
    private IceInternal.Buffer _readBuffer;
    private int _readBufferPos;
    private bool _buffered;
}
