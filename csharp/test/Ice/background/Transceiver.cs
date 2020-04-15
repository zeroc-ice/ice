//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;

using System;
using System.Collections.Generic;
using Test;
using System.Net.Sockets;

internal class Transceiver : IceInternal.ITransceiver
{
    public Socket? Fd() => _transceiver.Fd();

    public int Initialize(ref ArraySegment<byte> readBuffer, IList<ArraySegment<byte>> writeBuffer)
    {
        _configuration.CheckInitializeException();
        if (!_initialized)
        {
            int status = _transceiver.Initialize(ref readBuffer, writeBuffer);
            if (status != IceInternal.SocketOperation.None)
            {
                return status;
            }
            _initialized = true;
        }
        return IceInternal.SocketOperation.None;
    }

    public int Closing(bool initiator, System.Exception? ex) => _transceiver.Closing(initiator, ex);

    public void Close() => _transceiver.Close();

    public Ice.Endpoint Bind() => _transceiver.Bind();

    public int Write(IList<ArraySegment<byte>> buf, ref int offset)
    {
        int remaining = buf.GetByteCount() - offset;
        if (!_configuration.WriteReady() && remaining > 0)
        {
            return IceInternal.SocketOperation.Write;
        }

        _configuration.CheckWriteException();
        return _transceiver.Write(buf, ref offset);
    }

    public int Read(ref ArraySegment<byte> buffer, ref int offset)
    {
        if (!_configuration.ReadReady() && offset < buffer.Count)
        {
            return IceInternal.SocketOperation.Read;
        }

        _configuration.CheckReadException();

        if (_buffered)
        {
            while (offset < buffer.Count)
            {
                if (_readBufferPos == _readBufferOffset)
                {
                    _readBufferPos = 0;
                    _readBufferOffset = 0;
                    _transceiver.Read(ref _readBuffer, ref _readBufferOffset);
                    if (_readBufferPos == _readBufferOffset)
                    {
                        return IceInternal.SocketOperation.Read;
                    }
                }

                TestHelper.Assert(_readBufferOffset > _readBufferPos);
                int requested = buffer.Count - offset;
                int available = _readBufferOffset - _readBufferPos;
                TestHelper.Assert(available > 0);
                if (available >= requested)
                {
                    available = requested;
                }
                _readBuffer.Slice(_readBufferPos, available).CopyTo(buffer.Slice(offset));
                offset += available;
                _readBufferPos += available;
            }
            return IceInternal.SocketOperation.None;
        }
        else
        {
            return _transceiver.Read(ref buffer, ref offset);
        }
    }

    public bool StartRead(ref ArraySegment<byte> buffer, ref int offset, IceInternal.AsyncCallback callback,
        object state)
    {
        if (_configuration.ReadReady())
        {
            _configuration.CheckReadException(); // Only raise if we're configured to read now.
        }

        if (_buffered)
        {
            int available = _readBufferOffset - _readBufferPos;
            if (available > 0)
            {
                int requested = buffer.Count - offset;
                if (available >= requested)
                {
                    available = requested;
                }

                _readBuffer.Slice(_readBufferPos, available).CopyTo(buffer.Slice(offset));
                offset += available;
                _readBufferPos += available;
            }

            if (_readBufferPos == _readBufferOffset && offset < buffer.Count)
            {
                _readBufferPos = 0;
                _readBufferOffset = 0;
                return _transceiver.StartRead(ref _readBuffer, ref _readBufferOffset, callback, state);
            }
            else
            {
                TestHelper.Assert(offset == buffer.Count);
                return true; // Completed synchronously
            }
        }
        else
        {
            return _transceiver.StartRead(ref buffer, ref offset, callback, state);
        }
    }

    public void FinishRead(ref ArraySegment<byte> buffer, ref int offset)
    {
        _configuration.CheckReadException();
        if (_buffered)
        {
            if (offset < buffer.Count)
            {
                _transceiver.FinishRead(ref _readBuffer, ref _readBufferOffset);

                int requested = buffer.Count - offset;
                int available = _readBufferOffset - _readBufferPos;
                if (available > 0)
                {
                    if (available >= requested)
                    {
                        available = requested;
                    }
                    _readBuffer.Slice(_readBufferPos, available).CopyTo(buffer.Slice(offset));
                    offset += available;
                    _readBufferPos += available;
                }
            }
        }
        else
        {
            _transceiver.FinishRead(ref buffer, ref offset);
        }
    }

    public bool StartWrite(IList<ArraySegment<byte>> buf, int offset, IceInternal.AsyncCallback callback,
        object state, out bool completed)
    {
        _configuration.CheckWriteException();
        return _transceiver.StartWrite(buf, offset, callback, state, out completed);
    }

    public void FinishWrite(IList<ArraySegment<byte>> buf, ref int offset)
    {
        _configuration.CheckWriteException();
        _transceiver.FinishWrite(buf, ref offset);
    }

    public string Transport() => "test-" + _transceiver.Transport();

    public ConnectionInfo GetInfo() => _transceiver.GetInfo();

    public override string? ToString() => _transceiver.ToString();

    public string ToDetailedString() => _transceiver.ToDetailedString();

    public void CheckSendSize(int sz) => _transceiver.CheckSendSize(sz);

    public void SetBufferSize(int rcvSize, int sndSize) => _transceiver.SetBufferSize(rcvSize, sndSize);

    public void Destroy() => _transceiver.Destroy();

    public IceInternal.ITransceiver GetDelegate() => _transceiver;

    //
    // Only for use by Connector, Acceptor
    //
    internal Transceiver(IceInternal.ITransceiver transceiver)
    {
        _transceiver = transceiver;
        _configuration = Configuration.GetInstance();
        _initialized = false;
        _readBuffer = new byte[1024 * 8]; // 8KB buffer
        _readBufferOffset = 0;
        _readBufferPos = 0;
        _buffered = _configuration.Buffered();
    }

    private readonly IceInternal.ITransceiver _transceiver;
    private readonly Configuration _configuration;
    private bool _initialized;
    private ArraySegment<byte> _readBuffer;
    private int _readBufferOffset;
    private int _readBufferPos;
    private readonly bool _buffered;
}
