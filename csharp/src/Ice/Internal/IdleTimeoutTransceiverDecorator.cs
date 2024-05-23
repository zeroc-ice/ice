// Copyright (c) ZeroC, Inc. All rights reserved.

#nullable enable

using System.Net.Sockets;

namespace Ice.Internal;

internal sealed class IdleTimeoutTransceiverDecorator : Transceiver
{
    private readonly Transceiver _decoratee;
    private readonly TimeSpan _idleTimeout;
    private readonly bool _enableIdleCheck;

    public EndpointI bind() => _decoratee.bind();

    public void checkSendSize(Buffer buf) => _decoratee.checkSendSize(buf);

    public void close() => _decoratee.close();

    public int closing(bool initiator, LocalException ex) => _decoratee.closing(initiator, ex);

    public void destroy() => _decoratee.destroy();

    public Socket fd() => _decoratee.fd();

    public void finishRead(Buffer buf) => _decoratee.finishRead(buf);

    public void finishWrite(Buffer buf) => _decoratee.finishWrite(buf);

    public ConnectionInfo getInfo() => _decoratee.getInfo();

    public int initialize(Buffer readBuffer, Buffer writeBuffer, ref bool hasMoreData) =>
        _decoratee.initialize(readBuffer, writeBuffer, ref hasMoreData);

    public string protocol() => _decoratee.protocol();

    public int read(Buffer buf, ref bool hasMoreData) => _decoratee.read(buf, ref hasMoreData);

    public void setBufferSize(int rcvSize, int sndSize) => _decoratee.setBufferSize(rcvSize, sndSize);

    public bool startRead(Buffer buf, AsyncCallback callback, object state) =>
        _decoratee.startRead(buf, callback, state);

    public bool startWrite(Buffer buf, AsyncCallback callback, object state, out bool completed) =>
        _decoratee.startWrite(buf, callback, state, out completed);

    public string toDetailedString() => _decoratee.toDetailedString();

    public int write(Buffer buf) => _decoratee.write(buf);

    internal IdleTimeoutTransceiverDecorator(Transceiver decoratee, TimeSpan idleTimeout, bool enableIdleCheck)
    {
        _decoratee = decoratee;
        _idleTimeout = idleTimeout;
        _enableIdleCheck = enableIdleCheck;
    }
}
