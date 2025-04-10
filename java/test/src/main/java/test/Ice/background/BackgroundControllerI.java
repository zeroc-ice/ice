// Copyright (c) ZeroC, Inc.

package test.Ice.background;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.SocketException;

import test.Ice.background.Test.BackgroundController;

import java.util.HashSet;
import java.util.Set;

class BackgroundControllerI implements BackgroundController {
    @Override
    public synchronized void pauseCall(String opName, Current current) {
        _pausedCalls.add(opName);
    }

    @Override
    public synchronized void resumeCall(String opName, Current current) {
        _pausedCalls.remove(opName);
        notifyAll();
    }

    public synchronized void checkCallPause(Current current) {
        while (_pausedCalls.contains(current.operation)) {
            try {
                wait();
                break;
            } catch (InterruptedException ex) {}
        }
    }

    @Override
    public void holdAdapter(Current current) {
        _adapter.hold();
    }

    @Override
    public void resumeAdapter(Current current) {
        _adapter.activate();
    }

    @Override
    public void initializeSocketStatus(int status, Current current) {
        _configuration.initializeSocketStatus(status);
    }

    @Override
    public void initializeException(boolean enable, Current current) {
        _configuration.initializeException(enable ? new SocketException() : null);
    }

    @Override
    public void readReady(boolean enable, Current current) {
        _configuration.readReady(enable);
    }

    @Override
    public void readException(boolean enable, Current current) {
        _configuration.readException(enable ? new SocketException() : null);
    }

    @Override
    public void writeReady(boolean enable, Current current) {
        _configuration.writeReady(enable);
    }

    @Override
    public void writeException(boolean enable, Current current) {
        _configuration.writeException(enable ? new SocketException() : null);
    }

    @Override
    public void buffered(boolean enable, Current current) {
        _configuration.buffered(enable);
    }

    public BackgroundControllerI(Configuration configuration, ObjectAdapter adapter) {
        _adapter = adapter;
        _configuration = configuration;
    }

    private final ObjectAdapter _adapter;
    private final Set<String> _pausedCalls = new HashSet<>();
    private final Configuration _configuration;
}
