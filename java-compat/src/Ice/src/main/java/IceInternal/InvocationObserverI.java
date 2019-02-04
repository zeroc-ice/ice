//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

import IceMX.*;

public class InvocationObserverI
    extends IceMX.ObserverWithDelegate<IceMX.InvocationMetrics, Ice.Instrumentation.InvocationObserver>
    implements Ice.Instrumentation.InvocationObserver
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

        RemoteInvocationHelper(Ice.ConnectionInfo con, Ice.Endpoint endpt, int requestId, int size)
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

        public Ice.ConnectionInfo
        getConnectionInfo()
        {
            return _connectionInfo;
        }

        public Ice.Endpoint
        getEndpoint()
        {
            return _endpoint;
        }

        public Ice.EndpointInfo
        getEndpointInfo()
        {
            if(_endpointInfo == null)
            {
                _endpointInfo = _endpoint.getInfo();
            }
            return _endpointInfo;
        }

        final private Ice.ConnectionInfo _connectionInfo;
        final private Ice.Endpoint _endpoint;
        final private int _requestId;
        final private int _size;
        private String _id;
        private Ice.EndpointInfo _endpointInfo;
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

        CollocatedInvocationHelper(Ice.ObjectAdapter adapter, int requestId, int size)
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
    public Ice.Instrumentation.RemoteObserver
    getRemoteObserver(Ice.ConnectionInfo con, Ice.Endpoint edpt, int requestId, int sz)
    {
        Ice.Instrumentation.RemoteObserver delegate = null;
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
    public Ice.Instrumentation.CollocatedObserver
    getCollocatedObserver(Ice.ObjectAdapter adapter, int requestId, int sz)
    {
        Ice.Instrumentation.CollocatedObserver delegate = null;
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
