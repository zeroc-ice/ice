// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

// Decorates Transceiver to send heartbeats and optionally detect when no byte is received/read for
// a while. This decorator must not be applied on UDP connections.
final class IdleTimeoutTransceiverDecorator implements Transceiver {
    private final Transceiver _decoratee;
    private final int _idleTimeout;
    private final ScheduledExecutorService _scheduledExecutorService;

    // Protected by ConnectionI's mutex.
    private boolean _idleCheckEnabled;

    private final Runnable _idleCheck;
    private final Runnable _sendHeartbeat;

    // All calls to a transceiver are serialized by the parent ConnectionI's lock.
    private ScheduledFuture<?> _readTimerFuture;
    private ScheduledFuture<?> _writeTimerFuture;

    @Override
    public java.nio.channels.SelectableChannel fd() {
        return _decoratee.fd();
    }

    @Override
    public void setReadyCallback(ReadyCallback callback) {
        _decoratee.setReadyCallback(callback);
    }

    @Override
    public int initialize(Buffer readBuffer, Buffer writeBuffer) {
        return _decoratee.initialize(readBuffer, writeBuffer);
    }

    @Override
    public int closing(boolean initiator, LocalException ex) {
        return _decoratee.closing(initiator, ex);
    }

    @Override
    public void close() {
        disableIdleCheck();
        cancelWriteTimer();
        _decoratee.close();
    }

    @Override
    public EndpointI bind() {
        return _decoratee.bind();
    }

    @Override
    public int write(Buffer buf) {
        cancelWriteTimer();
        int op = _decoratee.write(buf);
        if (op == SocketOperation.None) { // write completed
            rescheduleWriteTimer();
        }
        return op;
    }

    @Override
    public int read(Buffer buf) {
        if (_idleCheckEnabled) {
            // We don't want the idle check to run while we're reading, so we reschedule it before
            // reading.
            rescheduleReadTimer();
        }
        return _decoratee.read(buf);
    }

    @Override
    public String protocol() {
        return _decoratee.protocol();
    }

    @Override
    public String toString() {
        return _decoratee.toString();
    }

    @Override
    public String toDetailedString() {
        return _decoratee.toDetailedString();
    }

    @Override
    public ConnectionInfo getInfo(boolean incoming, String adapterName, String connectionId) {
        return _decoratee.getInfo(incoming, adapterName, connectionId);
    }

    @Override
    public void checkSendSize(Buffer buf) {
        _decoratee.checkSendSize(buf);
    }

    @Override
    public void setBufferSize(int rcvSize, int sndSize) {
        _decoratee.setBufferSize(rcvSize, sndSize);
    }

    IdleTimeoutTransceiverDecorator(
            Transceiver decoratee,
            ConnectionI connection,
            int idleTimeout,
            boolean enableIdleCheck,
            ScheduledExecutorService scheduledExecutorService) {
        _decoratee = decoratee;
        _idleTimeout = idleTimeout;
        _scheduledExecutorService = scheduledExecutorService;

        _idleCheck = enableIdleCheck ? () -> connection.idleCheck(idleTimeout) : null;
        _sendHeartbeat = () -> connection.sendHeartbeat();
    }

    boolean isIdleCheckEnabled() {
        return _idleCheckEnabled;
    }

    void enableIdleCheck() {
        if (!_idleCheckEnabled && _idleCheck != null) {
            rescheduleReadTimer();
            _idleCheckEnabled = true;
        }
    }

    void disableIdleCheck() {
        if (_idleCheckEnabled && _idleCheck != null) {
            cancelReadTimer();
            _idleCheckEnabled = false;
        }
    }

    void scheduleHeartbeat() {
        // Reschedule because the connection establishment may have already written to the
        // connection and scheduled a heartbeat.
        rescheduleWriteTimer();
    }

    private void cancelReadTimer() {
        if (_readTimerFuture != null) {
            _readTimerFuture.cancel(false);
            _readTimerFuture = null;
        }
    }

    private void cancelWriteTimer() {
        if (_writeTimerFuture != null) {
            _writeTimerFuture.cancel(false);
            _writeTimerFuture = null;
        }
    }

    private void rescheduleReadTimer() {
        if (_idleCheck != null) {
            cancelReadTimer();
            _readTimerFuture =
                    _scheduledExecutorService.schedule(_idleCheck, _idleTimeout, TimeUnit.SECONDS);
        }
    }

    private void rescheduleWriteTimer() {
        cancelWriteTimer();
        _writeTimerFuture =
                _scheduledExecutorService.schedule(
                        _sendHeartbeat, _idleTimeout * 1000 / 2, TimeUnit.MILLISECONDS);
    }
}
