//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

import com.zeroc.IceMX.*;

public class InvocationObserverI
    extends com.zeroc.IceMX.ObserverWithDelegate<com.zeroc.IceMX.InvocationMetrics,
                                                 com.zeroc.Ice.Instrumentation.InvocationObserver>
    implements com.zeroc.Ice.Instrumentation.InvocationObserver
{
    static public final class RemoteInvocationHelper extends MetricsHelper<RemoteMetrics>
    {
        static private final AttributeResolver _attributes = new AttributeResolver()
        {
            {
                try
                {
                    Class<?> cl = RemoteInvocationHelper.class;
                    add("parent", cl.getDeclaredMethod("getParent"));
                    add("id", cl.getDeclaredMethod("getId"));
                    add("requestId", cl.getDeclaredMethod("getRequestId"));
                    CommunicatorObserverI.addConnectionAttributes(this, RemoteInvocationHelper.class);
                }
                catch(Exception ex)
                {
                    ex.printStackTrace();
                    assert(false);
                }
            }
        };

        RemoteInvocationHelper(com.zeroc.Ice.ConnectionInfo con, com.zeroc.Ice.Endpoint endpt, int requestId, int size)
        {
            super(_attributes);
            _connectionInfo = con;
            _endpoint = endpt;
            _requestId = requestId;
            _size = size;
        }

        @Override
        public void
        initMetrics(RemoteMetrics v)
        {
            v.size += _size;
        }

        public String
        getId()
        {
            if(_id == null)
            {
                _id = _endpoint.toString();
                if(_connectionInfo.connectionId != null && !_connectionInfo.connectionId.isEmpty())
                {
                    _id += " [" + _connectionInfo.connectionId + "]";
                }
            }
            return _id;
        }

        int
        getRequestId()
        {
            return _requestId;
        }

        public String
        getParent()
        {
            if(_connectionInfo.adapterName != null && !_connectionInfo.adapterName.isEmpty())
            {
                return _connectionInfo.adapterName;
            }
            else
            {
                return "Communicator";
            }
        }

        public com.zeroc.Ice.ConnectionInfo
        getConnectionInfo()
        {
            return _connectionInfo;
        }

        public com.zeroc.Ice.Endpoint
        getEndpoint()
        {
            return _endpoint;
        }

        public com.zeroc.Ice.EndpointInfo
        getEndpointInfo()
        {
            if(_endpointInfo == null)
            {
                _endpointInfo = _endpoint.getInfo();
            }
            return _endpointInfo;
        }

        final private com.zeroc.Ice.ConnectionInfo _connectionInfo;
        final private com.zeroc.Ice.Endpoint _endpoint;
        final private int _requestId;
        final private int _size;
        private String _id;
        private com.zeroc.Ice.EndpointInfo _endpointInfo;
    }

    static public final class CollocatedInvocationHelper extends MetricsHelper<CollocatedMetrics>
    {
        static private final AttributeResolver _attributes = new AttributeResolver()
        {
            {
                try
                {
                    Class<?> cl = CollocatedInvocationHelper.class;
                    add("parent", cl.getDeclaredMethod("getParent"));
                    add("id", cl.getDeclaredMethod("getId"));
                    add("requestId", cl.getDeclaredMethod("getRequestId"));
                }
                catch(Exception ex)
                {
                    ex.printStackTrace();
                    assert(false);
                }
            }
        };

        CollocatedInvocationHelper(com.zeroc.Ice.ObjectAdapter adapter, int requestId, int size)
        {
            super(_attributes);
            _id = adapter.getName();
            _requestId = requestId;
            _size = size;
        }

        @Override
        public void
        initMetrics(CollocatedMetrics v)
        {
            v.size += _size;
        }

        public String
        getId()
        {
            return _id;
        }

        int
        getRequestId()
        {
            return _requestId;
        }

        public String
        getParent()
        {
            return "Communicator";
        }

        final private int _requestId;
        final private int _size;
        final private String _id;
    }

    @Override
    public void
    userException()
    {
        forEach(_userException);
        if(_delegate != null)
        {
            _delegate.userException();
        }
    }

    @Override
    public void
    retried()
    {
        forEach(_incrementRetry);
        if(_delegate != null)
        {
            _delegate.retried();
        }
    }

    @Override
    public com.zeroc.Ice.Instrumentation.RemoteObserver
    getRemoteObserver(com.zeroc.Ice.ConnectionInfo con, com.zeroc.Ice.Endpoint edpt, int requestId, int sz)
    {
        com.zeroc.Ice.Instrumentation.RemoteObserver delegate = null;
        if(_delegate != null)
        {
            delegate = _delegate.getRemoteObserver(con, edpt, requestId, sz);
        }
        return getObserver("Remote",
                           new RemoteInvocationHelper(con, edpt, requestId, sz),
                           RemoteMetrics.class,
                           RemoteObserverI.class,
                           delegate);
    }

    @Override
    public com.zeroc.Ice.Instrumentation.CollocatedObserver
    getCollocatedObserver(com.zeroc.Ice.ObjectAdapter adapter, int requestId, int sz)
    {
        com.zeroc.Ice.Instrumentation.CollocatedObserver delegate = null;
        if(_delegate != null)
        {
            delegate = _delegate.getCollocatedObserver(adapter, requestId, sz);
        }
        return getObserver("Collocated",
                           new CollocatedInvocationHelper(adapter, requestId, sz),
                           CollocatedMetrics.class,
                           CollocatedObserverI.class,
                           delegate);
    }

    final MetricsUpdate<InvocationMetrics> _incrementRetry = new MetricsUpdate<InvocationMetrics>()
    {
        @Override
        public void
        update(InvocationMetrics v)
        {
            ++v.retry;
        }
    };

    final MetricsUpdate<InvocationMetrics> _userException = new MetricsUpdate<InvocationMetrics>()
    {
        @Override
        public void
        update(InvocationMetrics v)
        {
            ++v.userException;
        }
    };
}
