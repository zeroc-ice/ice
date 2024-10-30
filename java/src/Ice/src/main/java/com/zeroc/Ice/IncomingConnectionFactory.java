//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

final class IncomingConnectionFactory extends EventHandler implements ConnectionI.StartCallback {
    public synchronized void startAcceptor() {
        if (_state >= StateClosed || _acceptorStarted) {
            return;
        }

        try {
            createAcceptor();
        } catch (Exception ex) {
            String s =
                    "acceptor creation failed:\n"
                            + ex.getCause().getMessage()
                            + '\n'
                            + _acceptor.toString();
            _instance.initializationData().logger.error(s);
            _instance
                    .timer()
                    .schedule(() -> startAcceptor(), 1, java.util.concurrent.TimeUnit.SECONDS);
        }
    }

    public synchronized void activate() {
        setState(StateActive);
    }

    public synchronized void hold() {
        setState(StateHolding);
    }

    public synchronized void destroy() {
        setState(StateClosed);
    }

    public synchronized void updateConnectionObservers() {
        for (ConnectionI connection : _connections) {
            connection.updateObserver();
        }
    }

    public void waitUntilHolding() throws InterruptedException {
        java.util.LinkedList<ConnectionI> connections;

        synchronized (this) {
            //
            // First we wait until the connection factory itself is in holding
            // state.
            //
            while (_state < StateHolding) {
                wait();
            }

            //
            // We want to wait until all connections are in holding state
            // outside the thread synchronization.
            //
            connections = new java.util.LinkedList<>(_connections);
        }

        //
        // Now we wait until each connection is in holding state.
        //
        for (ConnectionI connection : connections) {
            connection.waitUntilHolding();
        }
    }

    public void waitUntilFinished() throws InterruptedException {
        java.util.LinkedList<ConnectionI> connections = null;

        synchronized (this) {
            //
            // First we wait until the factory is destroyed. If we are using
            // an acceptor, we also wait for it to be closed.
            //
            while (_state != StateFinished) {
                wait();
            }

            //
            // Clear the OA. See bug 1673 for the details of why this is necessary.
            //
            _adapter = null;

            //
            // We want to wait until all connections are finished outside the
            // thread synchronization.
            //
            connections = new java.util.LinkedList<>(_connections);
        }

        for (ConnectionI connection : connections) {
            try {
                connection.waitUntilFinished();
            } catch (InterruptedException e) {
                //
                // Force close all of the connections.
                //
                for (ConnectionI c : connections) {
                    c.abort();
                }
                throw e;
            }
        }

        synchronized (this) {
            _connections.clear();
        }
    }

    public synchronized EndpointI endpoint() {
        return _endpoint;
    }

    public synchronized java.util.LinkedList<ConnectionI> connections() {
        java.util.LinkedList<ConnectionI> connections = new java.util.LinkedList<>();

        //
        // Only copy connections which have not been destroyed.
        //
        for (ConnectionI connection : _connections) {
            if (connection.isActiveOrHolding()) {
                connections.add(connection);
            }
        }

        return connections;
    }

    public void flushAsyncBatchRequests(
            CompressBatch compressBatch, CommunicatorFlushBatch outAsync) {
        for (ConnectionI c :
                connections()) // connections() is synchronized, no need to synchronize here.
        {
            try {
                outAsync.flushConnection(c, compressBatch);
            } catch (LocalException ex) {
                // Ignore.
            }
        }
    }

    //
    // Operations from EventHandler.
    //

    @Override
    public void message(ThreadPoolCurrent current) {
        ConnectionI connection = null;
        synchronized (this) {
            if (_state >= StateClosed) {
                return;
            } else if (_state == StateHolding) {
                Thread.yield();
                return;
            }

            if (!_acceptorStarted) {
                return;
            }

            //
            // Now accept a new connection.
            //
            Transceiver transceiver = null;
            try {
                transceiver = _acceptor.accept();

                if (_maxConnections > 0 && _connections.size() == _maxConnections) {
                    // Can't accept more connections, so we abort this transport connection.
                    if (_instance.traceLevels().network >= 2) {
                        StringBuffer s = new StringBuffer("rejecting new ");
                        s.append(_endpoint.protocol());
                        s.append(" connection\n");
                        s.append(transceiver.toString());
                        s.append("\nbecause the maximum number of connections has been reached");
                        _instance
                                .initializationData()
                                .logger
                                .trace(_instance.traceLevels().networkCat, s.toString());
                    }
                    try {
                        transceiver.close();
                    } catch (com.zeroc.Ice.SocketException ex) {
                        // Ignore
                    }
                    return;
                }

                if (_instance.traceLevels().network >= 2) {
                    StringBuffer s = new StringBuffer("trying to accept ");
                    s.append(_endpoint.protocol());
                    s.append(" connection\n");
                    s.append(transceiver.toString());
                    _instance
                            .initializationData()
                            .logger
                            .trace(_instance.traceLevels().networkCat, s.toString());
                }
            } catch (com.zeroc.Ice.SocketException ex) {
                if (Network.noMoreFds(ex.getCause())) {
                    try {
                        String s = "can't accept more connections:\n" + ex.getCause().getMessage();
                        s += '\n' + _acceptor.toString();
                        try {
                            _instance.initializationData().logger.error(s);
                        } catch (Throwable ex1) {
                            System.out.println(s);
                        }
                    } catch (Throwable ex2) {
                        // Ignore, could be a class loading error.
                    }

                    assert (_acceptorStarted);
                    _acceptorStarted = false;
                    if (_adapter.getThreadPool().finish(this, true)) {
                        closeAcceptor();
                    }
                }

                // Ignore socket exceptions.
                return;
            } catch (LocalException ex) {
                // Warn about other Ice local exceptions.
                if (_warn) {
                    warning(ex);
                }
                return;
            }

            assert (transceiver != null);

            try {
                connection =
                        new ConnectionI(
                                _adapter.getCommunicator(),
                                _instance,
                                transceiver,
                                null,
                                _endpoint,
                                _adapter,
                                this::removeConnection,
                                _connectionOptions);
            } catch (LocalException ex) {
                try {
                    transceiver.close();
                } catch (LocalException exc) {
                    // Ignore
                }

                if (_warn) {
                    warning(ex);
                }
                return;
            }

            _connections.add(connection);
        }

        assert (connection != null);
        connection.start(this);
    }

    @Override
    public synchronized void finished(ThreadPoolCurrent current, boolean close) {
        if (_state < StateClosed) {
            if (close) {
                closeAcceptor();
            }

            //
            // If the acceptor hasn't been explicitly stopped (which is the case if the acceptor got
            // closed
            // because of an unexpected error), try to restart the acceptor in 1 second.
            //
            _instance
                    .timer()
                    .schedule(() -> startAcceptor(), 1, java.util.concurrent.TimeUnit.SECONDS);
            return;
        }

        assert (_state >= StateClosed);
        setState(StateFinished);

        if (close) {
            closeAcceptor();
        }
    }

    @Override
    public synchronized String toString() {
        if (_transceiver != null) {
            return _transceiver.toString();
        }
        return _acceptor.toString();
    }

    @Override
    public java.nio.channels.SelectableChannel fd() {
        assert (_acceptor != null);
        return _acceptor.fd();
    }

    @Override
    public void setReadyCallback(ReadyCallback readyCallback) {
        if (_acceptor != null) {
            _acceptor.setReadyCallback(readyCallback);
        }
    }

    //
    // Operations from ConnectionI.StartCallback
    //
    @Override
    public synchronized void connectionStartCompleted(ConnectionI connection) {
        //
        // Initially, connections are in the holding state. If the factory is active
        // we activate the connection.
        //
        if (_state == StateActive) {
            connection.activate();
        }
    }

    @Override
    public void connectionStartFailed(ConnectionI connection, LocalException ex) {
        // Do not warn about connection exceptions here. The connection is not yet validated.
    }

    public IncomingConnectionFactory(Instance instance, EndpointI endpoint, ObjectAdapter adapter) {
        _instance = instance;
        _connectionOptions = instance.serverConnectionOptions(adapter.getName());

        // Meaningful only for non-datagram (non-UDP) connections.
        _maxConnections =
                endpoint.datagram()
                        ? 0
                        : instance.initializationData()
                                .properties
                                .getPropertyAsInt(adapter.getName() + ".MaxConnections");

        _endpoint = endpoint;
        _adapter = adapter;
        _warn =
                _instance.initializationData().properties.getPropertyAsInt("Ice.Warn.Connections")
                                > 0
                        ? true
                        : false;
        _state = StateHolding;
        _acceptorStarted = false;

        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        if (defaultsAndOverrides.overrideCompress.isPresent()) {
            _endpoint = _endpoint.compress(defaultsAndOverrides.overrideCompress.get());
        }

        try {
            _transceiver = _endpoint.transceiver();
            if (_transceiver != null) {
                // All this is for UDP "connections".
                if (_instance.traceLevels().network >= 2) {
                    StringBuffer s = new StringBuffer("attempting to bind to ");
                    s.append(_endpoint.protocol());
                    s.append(" socket\n");
                    s.append(_transceiver.toString());
                    _instance
                            .initializationData()
                            .logger
                            .trace(_instance.traceLevels().networkCat, s.toString());
                }
                _endpoint = _transceiver.bind();

                var connection =
                        new ConnectionI(
                                _adapter.getCommunicator(),
                                _instance,
                                _transceiver,
                                null,
                                _endpoint,
                                _adapter,
                                null,
                                _connectionOptions);
                connection.startAndWait();

                _connections.add(connection);
                assert _maxConnections == 0; // UDP so no max connections
            } else {
                createAcceptor();
            }
        } catch (Exception ex) {
            //
            // Clean up for finalizer.
            //
            if (_transceiver != null) {
                try {
                    _transceiver.close();
                } catch (LocalException e) {
                    // Here we ignore any exceptions in close().
                }
            }

            _state = StateFinished;
            _connections.clear();

            if (ex instanceof LocalException) {
                throw (LocalException) ex;
            } else if (ex instanceof InterruptedException) {
                throw new OperationInterruptedException();
            } else {
                throw new SyscallException(ex);
            }
        }
    }

    @SuppressWarnings("deprecation")
    @Override
    protected synchronized void finalize() throws Throwable {
        try {
            Assert.FinalizerAssert(_state == StateFinished);
            Assert.FinalizerAssert(_connections.isEmpty());
        } catch (Exception ex) {
        } finally {
            super.finalize();
        }
    }

    private static final int StateActive = 0;
    private static final int StateHolding = 1;
    private static final int StateClosed = 2;
    private static final int StateFinished = 3;

    private void setState(int state) {
        if (_state == state) // Don't switch twice.
        {
            return;
        }

        switch (state) {
            case StateActive:
                {
                    if (_state != StateHolding) // Can only switch from holding to active.
                    {
                        return;
                    }
                    if (_acceptor != null) {
                        if (_instance.traceLevels().network >= 1) {
                            StringBuffer s = new StringBuffer("accepting ");
                            s.append(_endpoint.protocol());
                            s.append(" connections at ");
                            s.append(_acceptor.toString());
                            _instance
                                    .initializationData()
                                    .logger
                                    .trace(_instance.traceLevels().networkCat, s.toString());
                        }
                        _adapter.getThreadPool().register(this, SocketOperation.Read);
                    }

                    for (ConnectionI connection : _connections) {
                        connection.activate();
                    }
                    break;
                }

            case StateHolding:
                {
                    if (_state != StateActive) // Can only switch from active to holding.
                    {
                        return;
                    }
                    if (_acceptor != null) {
                        // Stop accepting new connections.
                        if (_instance.traceLevels().network >= 1) {
                            StringBuffer s = new StringBuffer("holding ");
                            s.append(_endpoint.protocol());
                            s.append(" connections at ");
                            s.append(_acceptor.toString());
                            _instance
                                    .initializationData()
                                    .logger
                                    .trace(_instance.traceLevels().networkCat, s.toString());
                        }
                        _adapter.getThreadPool().unregister(this, SocketOperation.Read);
                    }

                    for (ConnectionI connection : _connections) {
                        connection.hold();
                    }
                    break;
                }

            case StateClosed:
                {
                    if (_acceptorStarted) {
                        //
                        // If possible, close the acceptor now to prevent new connections from
                        // being accepted while we are deactivating. This is especially useful
                        // if there are no more threads in the thread pool available to dispatch
                        // the finish() call.
                        //
                        _acceptorStarted = false;
                        if (_adapter.getThreadPool().finish(this, true)) {
                            closeAcceptor();
                        }
                    } else {
                        state = StateFinished;
                    }

                    for (ConnectionI connection : _connections) {
                        connection.destroy(ConnectionI.ObjectAdapterDeactivated);
                    }
                    break;
                }

            case StateFinished:
                {
                    assert (_state == StateClosed);
                    break;
                }
        }

        _state = state;
        notifyAll();
    }

    private void createAcceptor() {
        try {
            assert (!_acceptorStarted);
            _acceptor = _endpoint.acceptor(_adapter.getName(), _adapter.getSSLEngineFactory());
            assert (_acceptor != null);

            if (_instance.traceLevels().network >= 2) {
                StringBuffer s = new StringBuffer("attempting to bind to ");
                s.append(_endpoint.protocol());
                s.append(" socket ");
                s.append(_acceptor.toString());
                _instance
                        .initializationData()
                        .logger
                        .trace(_instance.traceLevels().networkCat, s.toString());
            }

            _endpoint = _acceptor.listen();

            if (_instance.traceLevels().network >= 1) {
                StringBuffer s = new StringBuffer("listening for ");
                s.append(_endpoint.protocol());
                s.append(" connections\n");
                s.append(_acceptor.toDetailedString());
                _instance
                        .initializationData()
                        .logger
                        .trace(_instance.traceLevels().networkCat, s.toString());
            }

            _adapter.getThreadPool().initialize(this);

            if (_state == StateActive) {
                _adapter.getThreadPool().register(this, SocketOperation.Read);
            }

            _acceptorStarted = true;
        } catch (Exception ex) {
            if (_acceptor != null) {
                _acceptor.close();
            }
            throw ex;
        }
    }

    private void closeAcceptor() {
        assert (_acceptor != null);

        if (_instance.traceLevels().network >= 1) {
            StringBuffer s = new StringBuffer("stopping to accept ");
            s.append(_endpoint.protocol());
            s.append(" connections at ");
            s.append(_acceptor.toString());
            _instance
                    .initializationData()
                    .logger
                    .trace(_instance.traceLevels().networkCat, s.toString());
        }

        assert (!_acceptorStarted);
        _acceptor.close();
    }

    private synchronized void removeConnection(ConnectionI connection) {
        if (_state == StateActive || _state == StateHolding) {
            _connections.remove(connection);
        }
        // else it's already being cleaned up.
    }

    private void warning(LocalException ex) {
        String s = "connection exception:\n" + Ex.toString(ex) + '\n' + _acceptor.toString();
        _instance.initializationData().logger.warning(s);
    }

    private final Instance _instance;
    private final ConnectionOptions _connectionOptions;

    private final int _maxConnections;

    private Acceptor _acceptor;
    private Transceiver _transceiver;
    private EndpointI _endpoint;

    private ObjectAdapter _adapter;

    private final boolean _warn;

    private java.util.Set<ConnectionI> _connections = new java.util.HashSet<>();

    private int _state;
    private boolean _acceptorStarted;
}
