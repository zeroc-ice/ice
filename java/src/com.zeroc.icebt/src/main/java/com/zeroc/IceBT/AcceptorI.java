// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBT;

import com.zeroc.Ice.Acceptor;
import com.zeroc.Ice.ReadyCallback;
import com.zeroc.Ice.SocketException;
import com.zeroc.Ice.SocketOperation;
import com.zeroc.Ice.Transceiver;

import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;

import java.io.IOException;
import java.nio.channels.ServerSocketChannel;
import java.util.Stack;
import java.util.UUID;

final class AcceptorI implements Acceptor {
    @Override
    public ServerSocketChannel fd() {
        return null;
    }

    @Override
    public synchronized void setReadyCallback(ReadyCallback callback) {
        _readyCallback = callback;
        notify(); // Notify the acceptor thread
    }

    @Override
    public void close() {
        synchronized (this) {
            _closed = true;
        }

        if (_socket != null) {
            try {
                _socket.close(); // Wakes up the thread blocked in accept().
            } catch (Exception ex) {
                // Ignore.
            }
        }
        if (_thread != null) {
            try {
                _thread.join();
            } catch (Exception ex) {
                // Ignore.
            }
        }
    }

    @Override
    public com.zeroc.Ice.EndpointI listen() {
        UUID uuid = null;
        try {
            uuid = UUID.fromString(_uuid);
        } catch (IllegalArgumentException ex) {
            throw new SocketException(ex);
        }

        try {
            // We always listen using the "secure" method.
            _socket = _instance.bluetoothAdapter().listenUsingRfcommWithServiceRecord(_name, uuid);
        } catch (IOException ex) {
            throw new SocketException(ex);
        }

        // Use a helper thread to perform the blocking accept() calls.
        _thread =
            new Thread() {
                public void run() {
                    runAccept();
                }
            };
        _thread.start();

        return _endpoint;
    }

    @Override
    public synchronized Transceiver accept() {
        if (_exception != null) {
            throw new SocketException(_exception);
        }

        // accept() should only be called when we have at least one socket ready.
        assert (!_pending.isEmpty());

        BluetoothSocket socket = _pending.pop();

        // Update our status with the thread pool.
        _readyCallback.ready(SocketOperation.Read, !_pending.isEmpty());

        return new TransceiverI(_instance, socket, _uuid, _adapterName);
    }

    @Override
    public String protocol() {
        return _instance.protocol();
    }

    @Override
    public String toString() {
        StringBuffer s = new StringBuffer("local address = ");
        s.append(_instance.bluetoothAdapter().getAddress());
        return s.toString();
    }

    @Override
    public String toDetailedString() {
        StringBuffer s = new StringBuffer(toString());
        if (!_name.isEmpty()) {
            s.append("\nservice name = '");
            s.append(_name);
            s.append("'");
        }
        if (!_uuid.isEmpty()) {
            s.append("\nservice uuid = ");
            s.append(_uuid);
        }
        return s.toString();
    }

    AcceptorI(EndpointI endpoint, Instance instance, String adapterName, String uuid, String name) {
        _endpoint = endpoint;
        _instance = instance;
        _adapterName = adapterName;
        _name = name;
        _uuid = uuid;

        _pending = new Stack<BluetoothSocket>();
        _closed = false;
    }

    private void runAccept() {
        synchronized (this) {
            // Wait for the ready callback to be set by the selector.
            while (_readyCallback == null) {
                try {
                    wait();
                } catch (InterruptedException ex) {}
            }
        }

        while (true) {
            try {
                BluetoothSocket socket = _socket.accept();
                synchronized (this) {
                    _pending.push(socket);

                    // Notify the thread pool that we are ready to "read". The thread pool will
                    // invoke accept()
                    // and we can return a new transceiver.
                    _readyCallback.ready(SocketOperation.Read, true);
                }
            } catch (Exception ex) {
                synchronized (this) {
                    if (_closed) {
                        break;
                    }
                    _exception = ex;
                    _readyCallback.ready(SocketOperation.Read, true);
                }
            }
        }

        // Close any remaining incoming sockets that haven't been accepted yet.
        for (BluetoothSocket s : _pending) {
            try {
                s.close();
            } catch (Exception ex) {
                // Ignore.
            }
        }
        _pending.clear();
    }

    private final EndpointI _endpoint;
    private final Instance _instance;
    private final String _adapterName;
    private final String _name;
    private final String _uuid;
    private ReadyCallback _readyCallback;
    private BluetoothServerSocket _socket;
    private final Stack<BluetoothSocket> _pending;
    private Thread _thread;
    private Exception _exception;
    private boolean _closed;
}
