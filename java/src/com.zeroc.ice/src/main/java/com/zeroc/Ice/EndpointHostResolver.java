// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.Instrumentation.CommunicatorObserver;
import com.zeroc.Ice.Instrumentation.Observer;
import com.zeroc.Ice.Instrumentation.ThreadObserver;
import com.zeroc.Ice.Instrumentation.ThreadState;

import java.net.InetSocketAddress;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

class EndpointHostResolver {
    EndpointHostResolver(Instance instance) {
        _instance = instance;
        _protocol = instance.protocolSupport();
        _preferIPv6 = instance.preferIPv6();
        try {
            _threadName =
                    Util.createThreadName(
                            _instance.initializationData().properties, "Ice.HostResolver");
            _executor =
                    Executors.newFixedThreadPool(
                            1,
                            Util.createThreadFactory(
                                    _instance.initializationData().properties, _threadName));
            updateObserver();
        } catch (RuntimeException ex) {
            String s =
                    "cannot create thread for endpoint host resolver thread:\n" + Ex.toString(ex);
            _instance.initializationData().logger.error(s);
            throw ex;
        }
    }

    synchronized void resolve(
            final String host,
            final int port,
            final IPEndpointI endpoint,
            final EndpointI_connectors callback) {
        //
        // TODO: Optimize to avoid the lookup if the given host is a textual IPv4 or IPv6 address.
        // This requires implementing parsing of IPv4/IPv6 addresses (Java does not provide such
        // methods).
        //

        assert (!_destroyed);

        NetworkProxy networkProxy = _instance.networkProxy();
        if (networkProxy == null) {
            List<InetSocketAddress> addrs =
                    Network.getAddresses(host, port, _protocol, _preferIPv6, false);
            if (addrs != null) {
                callback.connectors(endpoint.connectors(addrs, networkProxy));
                return;
            }
        }

        final ThreadObserver threadObserver = _observer;
        final Observer observer = getObserver(endpoint);
        if (observer != null) {
            observer.attach();
        }

        _executor.execute(
                () -> {
                    synchronized (EndpointHostResolver.this) {
                        if (_destroyed) {
                            var ex = new CommunicatorDestroyedException();
                            if (observer != null) {
                                observer.failed(ex.ice_id());
                                observer.detach();
                            }
                            callback.exception(ex);
                            return;
                        }
                    }

                    if (threadObserver != null) {
                        threadObserver.stateChanged(
                                ThreadState.ThreadStateIdle, ThreadState.ThreadStateInUseForOther);
                    }

                    Observer obsv = observer;
                    try {
                        int protocol = _protocol;
                        NetworkProxy np = _instance.networkProxy();
                        if (np != null) {
                            np = np.resolveHost(_protocol);
                            if (np != null) {
                                protocol = np.getProtocolSupport();
                            }
                        }

                        List<InetSocketAddress> addresses =
                                Network.getAddresses(host, port, protocol, _preferIPv6, true);

                        if (obsv != null) {
                            obsv.detach();
                            obsv = null;
                        }

                        callback.connectors(endpoint.connectors(addresses, np));
                    } catch (LocalException ex) {
                        if (obsv != null) {
                            obsv.failed(ex.ice_id());
                            obsv.detach();
                        }
                        callback.exception(ex);
                    } finally {
                        if (threadObserver != null) {
                            threadObserver.stateChanged(
                                    ThreadState.ThreadStateInUseForOther,
                                    ThreadState.ThreadStateIdle);
                        }
                    }
                });
    }

    synchronized void destroy() {
        if (!_destroyed) {
            _destroyed = true;

            //
            // Shutdown the executor. No new tasks will be accepted. Existing tasks will execute.
            //
            _executor.shutdown();
        }
    }

    void joinWithThread() throws InterruptedException {
        // Wait for the executor to terminate.
        try {
            while (!_executor.isTerminated()) {
                // A very long time.
                _executor.awaitTermination(100000, TimeUnit.SECONDS);
            }

        } finally {
            if (_observer != null) {
                _observer.detach();
            }
        }
    }

    synchronized void updateObserver() {
        CommunicatorObserver obsv =
                _instance.initializationData().observer;
        if (obsv != null) {
            _observer =
                    obsv.getThreadObserver(
                            "Communicator", _threadName, ThreadState.ThreadStateIdle, _observer);
            if (_observer != null) {
                _observer.attach();
            }
        }
    }

    private Observer getObserver(IPEndpointI endpoint) {
        CommunicatorObserver obsv =
                _instance.initializationData().observer;
        if (obsv != null) {
            return obsv.getEndpointLookupObserver(endpoint);
        }
        return null;
    }

    private final Instance _instance;
    private final int _protocol;
    private final boolean _preferIPv6;
    private boolean _destroyed;
    private ThreadObserver _observer;
    private String _threadName;
    private ExecutorService _executor;
}
