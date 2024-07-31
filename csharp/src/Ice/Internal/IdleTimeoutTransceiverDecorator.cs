// Copyright (c) ZeroC, Inc. All rights reserved.

#nullable enable

using System.Diagnostics;
using System.Net.Sockets;

namespace Ice.Internal;

internal sealed class IdleTimeoutTransceiverDecorator : Transceiver
{
    public bool isWaitingToBeRead => _decoratee.isWaitingToBeRead;

    private readonly Transceiver _decoratee;
    private readonly TimeSpan _idleTimeout;
    private readonly System.Threading.Timer? _readTimer;
    private readonly System.Threading.Timer _writeTimer;

    public override string ToString() => _decoratee.ToString()!;

    public EndpointI? bind() => _decoratee.bind();

    public void checkSendSize(Buffer buf) => _decoratee.checkSendSize(buf);

    public void close()
    {
        cancelReadTimer();
        cancelWriteTimer();
        _decoratee.close();
    }

    public int closing(bool initiator, LocalException ex) => _decoratee.closing(initiator, ex);

    public void destroy()
    {
        cancelReadTimer();
        cancelWriteTimer();
        _decoratee.destroy();
        _readTimer?.Dispose();
        _writeTimer.Dispose();
    }

    public Socket fd() => _decoratee.fd();

    public void finishRead(Buffer buf)
    {
        // We don't want the idle check to run while we're reading, so we reschedule it before reading.
        rescheduleReadTimer();
        _decoratee.finishRead(buf);
    }

    // We call write after finishWrite, so no need to do anything here.
    public void finishWrite(Buffer buf) => _decoratee.finishWrite(buf);

    public ConnectionInfo getInfo() => _decoratee.getInfo();

    public int initialize(Buffer readBuffer, Buffer writeBuffer, ref bool hasMoreData)
    {
        int op = _decoratee.initialize(readBuffer, writeBuffer, ref hasMoreData);

        if (op == SocketOperation.None) // connected
        {
            rescheduleReadTimer();
            rescheduleWriteTimer();
        }

        return op;
    }

    public string protocol() => _decoratee.protocol();

    public int read(Buffer buf, ref bool hasMoreData)
    {
        // We don't want the idle check to run while we're reading, so we reschedule it before reading.
        rescheduleReadTimer();
        return _decoratee.read(buf, ref hasMoreData);
    }

    public void setBufferSize(int rcvSize, int sndSize) => _decoratee.setBufferSize(rcvSize, sndSize);

    // We always call finishRead or read to actually read the data.
    public bool startRead(Buffer buf, AsyncCallback callback, object state) =>
        _decoratee.startRead(buf, callback, state);

    // startWrite is called right after write returns SocketOperationWrite. The logic in write is sufficient.
    public bool startWrite(Buffer buf, AsyncCallback callback, object state, out bool completed) =>
        _decoratee.startWrite(buf, callback, state, out completed);

    public string toDetailedString() => _decoratee.toDetailedString();

    public int write(Buffer buf)
    {
        cancelWriteTimer();
        int op = _decoratee.write(buf);
        if (op == SocketOperation.None) // write completed
        {
            rescheduleWriteTimer();
        }
        return op;
    }

    internal IdleTimeoutTransceiverDecorator(Transceiver decoratee, ConnectionI connection, TimeSpan idleTimeout, bool enableIdleCheck)
    {
        Debug.Assert(idleTimeout > TimeSpan.Zero);

        _decoratee = decoratee;
        _idleTimeout = idleTimeout;

        if (enableIdleCheck)
        {
            _readTimer = new System.Threading.Timer(_ => connection.idleCheck(_idleTimeout, rescheduleReadTimer));
        }

        _writeTimer = new System.Threading.Timer(_ => connection.sendHeartbeat());
    }

    private void cancelReadTimer() => _readTimer?.Change(Timeout.InfiniteTimeSpan, Timeout.InfiniteTimeSpan);
    private void cancelWriteTimer() => _writeTimer.Change(Timeout.InfiniteTimeSpan, Timeout.InfiniteTimeSpan);
    private void rescheduleReadTimer() => _readTimer?.Change(_idleTimeout, Timeout.InfiniteTimeSpan);
    private void rescheduleWriteTimer() => _writeTimer.Change(_idleTimeout / 2, Timeout.InfiniteTimeSpan);
}
