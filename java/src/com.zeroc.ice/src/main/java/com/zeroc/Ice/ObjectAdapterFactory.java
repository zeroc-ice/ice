//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import com.zeroc.Ice.SSL.SSLEngineFactory;

final class ObjectAdapterFactory {
    public void shutdown() {
        java.util.List<ObjectAdapter> adapters;
        synchronized (this) {
            // Ignore shutdown requests if the object adapter factory has already been shut down.
            if (_instance == null) {
                return;
            }

            _instance = null;
            _communicator = null;
            notifyAll();

            adapters = new java.util.LinkedList<>(_adapters);
        }

        // Deactivate outside the thread synchronization, to avoid deadlocks.
        for (ObjectAdapter adapter : adapters) {
            adapter.deactivate();
        }
    }

    public void waitForShutdown() {
        java.util.List<ObjectAdapter> adapters;
        synchronized (this) {
            //
            // First we wait for the shutdown of the factory itself.
            //
            while (_instance != null) {
                try {
                    wait();
                } catch (InterruptedException ex) {
                    throw new OperationInterruptedException(ex);
                }
            }

            adapters = new java.util.LinkedList<>(_adapters);
        }

        //
        // Now we wait for deactivation of each object adapter.
        //
        for (ObjectAdapter adapter : adapters) {
            adapter.waitForDeactivate();
        }
    }

    public synchronized boolean isShutdown() {
        return _instance == null;
    }

    public void destroy() {
        //
        // First wait for shutdown to finish.
        //
        waitForShutdown();

        java.util.List<ObjectAdapter> adapters;
        synchronized (this) {
            adapters = new java.util.LinkedList<>(_adapters);
        }

        for (ObjectAdapter adapter : adapters) {
            adapter.destroy();
        }

        synchronized (this) {
            _adapters.clear();
        }
    }

    public void updateConnectionObservers() {
        java.util.List<ObjectAdapter> adapters;
        synchronized (this) {
            adapters = new java.util.LinkedList<>(_adapters);
        }

        for (ObjectAdapter adapter : adapters) {
            adapter.updateConnectionObservers();
        }
    }

    public void updateThreadObservers() {
        java.util.List<ObjectAdapter> adapters;
        synchronized (this) {
            adapters = new java.util.LinkedList<>(_adapters);
        }

        for (ObjectAdapter adapter : adapters) {
            adapter.updateThreadObservers();
        }
    }

    public ObjectAdapter createObjectAdapter(
            String name, RouterPrx router, SSLEngineFactory sslEngineFactory) {
        synchronized (this) {
            if (_instance == null) {
                throw new CommunicatorDestroyedException();
            }

            if (!name.isEmpty()) {
                if (_adapterNamesInUse.contains(name)) {
                    throw new AlreadyRegisteredException("object adapter", name);
                }
                _adapterNamesInUse.add(name);
            }
        }

        //
        // Must be called outside the synchronization since initialize can make client invocations
        // on the router if it's set.
        //
        ObjectAdapter adapter = null;
        try {
            if (name.isEmpty()) {
                String uuid = java.util.UUID.randomUUID().toString();
                adapter =
                        new ObjectAdapter(
                                _instance, _communicator, this, uuid, null, true, sslEngineFactory);
            } else {
                adapter =
                        new ObjectAdapter(
                                _instance,
                                _communicator,
                                this,
                                name,
                                router,
                                false,
                                sslEngineFactory);
            }

            synchronized (this) {
                if (_instance == null) {
                    throw new CommunicatorDestroyedException();
                }
                _adapters.add(adapter);
            }
        } catch (CommunicatorDestroyedException ex) {
            if (adapter != null) {
                adapter.destroy();
            }
            throw ex;
        } catch (LocalException ex) {
            if (!name.isEmpty()) {
                synchronized (this) {
                    _adapterNamesInUse.remove(name);
                }
            }
            throw ex;
        }

        return adapter;
    }

    public ObjectAdapter findObjectAdapter(Reference ref) {
        java.util.List<ObjectAdapter> adapters;
        synchronized (this) {
            if (_instance == null) {
                return null;
            }

            adapters = new java.util.LinkedList<>(_adapters);
        }

        for (ObjectAdapter adapter : adapters) {
            try {
                if (adapter.isLocal(ref)) {
                    return adapter;
                }
            } catch (ObjectAdapterDestroyedException ex) {
                // Ignore.
            }
        }

        return null;
    }

    public synchronized void removeObjectAdapter(ObjectAdapter adapter) {
        if (_instance == null) {
            return;
        }

        _adapters.remove(adapter);
        _adapterNamesInUse.remove(adapter.getName());
    }

    public void flushAsyncBatchRequests(
            CompressBatch compressBatch, CommunicatorFlushBatch outAsync) {
        java.util.List<ObjectAdapter> adapters;
        synchronized (this) {
            adapters = new java.util.LinkedList<>(_adapters);
        }

        for (ObjectAdapter adapter : adapters) {
            adapter.flushAsyncBatchRequests(compressBatch, outAsync);
        }
    }

    //
    // Only for use by Instance.
    //
    ObjectAdapterFactory(Instance instance, Communicator communicator) {
        _instance = instance;
        _communicator = communicator;
    }

    @SuppressWarnings("deprecation")
    @Override
    protected synchronized void finalize() throws Throwable {
        try {
            Assert.FinalizerAssert(_instance == null);
            Assert.FinalizerAssert(_communicator == null);
            Assert.FinalizerAssert(_adapters.isEmpty());
        } catch (Exception ex) {
        } finally {
            super.finalize();
        }
    }

    private Instance _instance;
    private Communicator _communicator;
    private java.util.Set<String> _adapterNamesInUse = new java.util.HashSet<>();
    private java.util.List<ObjectAdapter> _adapters = new java.util.LinkedList<>();
}
