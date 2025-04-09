// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import com.zeroc.Ice.IceMX.CollocatedMetrics;
import com.zeroc.Ice.IceMX.InvocationMetrics;
import com.zeroc.Ice.IceMX.MetricsHelper;
import com.zeroc.Ice.IceMX.MetricsHelper.AttributeResolver;
import com.zeroc.Ice.IceMX.Observer.MetricsUpdate;
import com.zeroc.Ice.IceMX.ObserverWithDelegate;
import com.zeroc.Ice.IceMX.RemoteMetrics;
import com.zeroc.Ice.Instrumentation.CollocatedObserver;
import com.zeroc.Ice.Instrumentation.InvocationObserver;
import com.zeroc.Ice.Instrumentation.RemoteObserver;

/**
 * @hidden Public because it's used by IceMX (via reflection).
 */
public class InvocationObserverI extends ObserverWithDelegate<InvocationMetrics, InvocationObserver>
    implements InvocationObserver {
    public static final class RemoteInvocationHelper extends MetricsHelper<RemoteMetrics> {
        private static final AttributeResolver _attributes =
            new AttributeResolver() {
                {
                    try {
                        Class<?> cl = RemoteInvocationHelper.class;
                        add("parent", cl.getDeclaredMethod("getParent"));
                        add("id", cl.getDeclaredMethod("getId"));
                        add("requestId", cl.getDeclaredMethod("getRequestId"));
                        CommunicatorObserverI.addConnectionAttributes(
                            this, RemoteInvocationHelper.class);
                    } catch (Exception ex) {
                        ex.printStackTrace();
                        assert false;
                    }
                }
            };

        RemoteInvocationHelper(ConnectionInfo con, Endpoint endpt, int requestId, int size) {
            super(_attributes);
            _connectionInfo = con;
            _endpoint = endpt;
            _requestId = requestId;
            _size = size;
        }

        @Override
        public void initMetrics(RemoteMetrics v) {
            v.size += _size;
        }

        public String getId() {
            if (_id == null) {
                _id = _endpoint.toString();
                if (_connectionInfo.connectionId != null
                    && !_connectionInfo.connectionId.isEmpty()) {
                    _id += " [" + _connectionInfo.connectionId + "]";
                }
            }
            return _id;
        }

        int getRequestId() {
            return _requestId;
        }

        public String getParent() {
            if (_connectionInfo.adapterName != null && !_connectionInfo.adapterName.isEmpty()) {
                return _connectionInfo.adapterName;
            } else {
                return "Communicator";
            }
        }

        public ConnectionInfo getConnectionInfo() {
            return _connectionInfo;
        }

        public Endpoint getEndpoint() {
            return _endpoint;
        }

        public EndpointInfo getEndpointInfo() {
            if (_endpointInfo == null) {
                _endpointInfo = _endpoint.getInfo();
            }
            return _endpointInfo;
        }

        private final ConnectionInfo _connectionInfo;
        private final Endpoint _endpoint;
        private final int _requestId;
        private final int _size;
        private String _id;
        private EndpointInfo _endpointInfo;
    }

    public static final class CollocatedInvocationHelper extends MetricsHelper<CollocatedMetrics> {
        private static final AttributeResolver _attributes =
            new AttributeResolver() {
                {
                    try {
                        Class<?> cl = CollocatedInvocationHelper.class;
                        add("parent", cl.getDeclaredMethod("getParent"));
                        add("id", cl.getDeclaredMethod("getId"));
                        add("requestId", cl.getDeclaredMethod("getRequestId"));
                    } catch (Exception ex) {
                        ex.printStackTrace();
                        assert false;
                    }
                }
            };

        CollocatedInvocationHelper(ObjectAdapter adapter, int requestId, int size) {
            super(_attributes);
            _id = adapter.getName();
            _requestId = requestId;
            _size = size;
        }

        @Override
        public void initMetrics(CollocatedMetrics v) {
            v.size += _size;
        }

        public String getId() {
            return _id;
        }

        int getRequestId() {
            return _requestId;
        }

        public String getParent() {
            return "Communicator";
        }

        private final int _requestId;
        private final int _size;
        private final String _id;
    }

    @Override
    public void userException() {
        forEach(_userException);
        if (_delegate != null) {
            _delegate.userException();
        }
    }

    @Override
    public void retried() {
        forEach(_incrementRetry);
        if (_delegate != null) {
            _delegate.retried();
        }
    }

    @Override
    public RemoteObserver getRemoteObserver(
            ConnectionInfo con, Endpoint edpt, int requestId, int sz) {
        RemoteObserver delegate = null;
        if (_delegate != null) {
            delegate = _delegate.getRemoteObserver(con, edpt, requestId, sz);
        }
        return getObserver(
            "Remote",
            new RemoteInvocationHelper(con, edpt, requestId, sz),
            RemoteMetrics.class,
            RemoteObserverI.class,
            delegate);
    }

    @Override
    public CollocatedObserver getCollocatedObserver(ObjectAdapter adapter, int requestId, int sz) {
        CollocatedObserver delegate = null;
        if (_delegate != null) {
            delegate = _delegate.getCollocatedObserver(adapter, requestId, sz);
        }
        return getObserver(
            "Collocated",
            new CollocatedInvocationHelper(adapter, requestId, sz),
            CollocatedMetrics.class,
            CollocatedObserverI.class,
            delegate);
    }

    final MetricsUpdate<InvocationMetrics> _incrementRetry =
        new MetricsUpdate<InvocationMetrics>() {
            @Override
            public void update(InvocationMetrics v) {
                ++v.retry;
            }
        };

    final MetricsUpdate<InvocationMetrics> _userException =
        new MetricsUpdate<InvocationMetrics>() {
            @Override
            public void update(InvocationMetrics v) {
                ++v.userException;
            }
        };
}
