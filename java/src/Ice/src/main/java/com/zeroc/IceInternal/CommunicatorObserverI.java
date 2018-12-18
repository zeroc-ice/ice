// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceInternal;

import com.zeroc.IceMX.*;

public class CommunicatorObserverI implements com.zeroc.Ice.Instrumentation.CommunicatorObserver
{
    static void
    addEndpointAttributes(MetricsHelper.AttributeResolver r, Class<?> cl)
        throws Exception
    {
        r.add("endpoint", cl.getDeclaredMethod("getEndpoint"));

        Class<?> cli = com.zeroc.Ice.EndpointInfo.class;
        r.add("endpointType", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredMethod("type"));
        r.add("endpointIsDatagram", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredMethod("datagram"));
        r.add("endpointIsSecure", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredMethod("secure"));
        r.add("endpointTimeout", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredField("timeout"));
        r.add("endpointCompress", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredField("compress"));

        cli = com.zeroc.Ice.IPEndpointInfo.class;
        r.add("endpointHost", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredField("host"));
        r.add("endpointPort", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredField("port"));
    }

    static void
    addConnectionAttributes(MetricsHelper.AttributeResolver r, Class<?> cl)
        throws Exception
    {
        Class<?> cli = com.zeroc.Ice.ConnectionInfo.class;
        r.add("incoming", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("incoming"));
        r.add("adapterName", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("adapterName"));
        r.add("connectionId", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("connectionId"));

        cli = com.zeroc.Ice.IPConnectionInfo.class;
        r.add("localHost", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("localAddress"));
        r.add("localPort", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("localPort"));
        r.add("remoteHost", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("remoteAddress"));
        r.add("remotePort", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("remotePort"));

        cli = com.zeroc.Ice.UDPConnectionInfo.class;
        r.add("mcastHost", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("mcastAddress"));
        r.add("mcastPort", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("mcastPort"));

        addEndpointAttributes(r, cl);
    }

    static public class ConnectionHelper extends MetricsHelper<ConnectionMetrics>
    {
        static private AttributeResolver _attributes = new AttributeResolver()
        {
            {
                try
                {
                    add("parent", ConnectionHelper.class.getDeclaredMethod("getParent"));
                    add("id", ConnectionHelper.class.getDeclaredMethod("getId"));
                    add("state", ConnectionHelper.class.getDeclaredMethod("getState"));
                    addConnectionAttributes(this, ConnectionHelper.class);
                }
                catch(Exception ex)
                {
                    ex.printStackTrace();
                    assert(false);
                }
            }
        };

        ConnectionHelper(com.zeroc.Ice.ConnectionInfo con, com.zeroc.Ice.Endpoint endpt,
                         com.zeroc.Ice.Instrumentation.ConnectionState state)
        {
            super(_attributes);
            _connectionInfo = con;
            _endpoint = endpt;
            _state = state;
        }

        public String
        getId()
        {
            if(_id == null)
            {
                StringBuilder os = new StringBuilder();
                com.zeroc.Ice.IPConnectionInfo info = getIPConnectionInfo();
                if(info != null)
                {
                    os.append(info.localAddress).append(':').append(info.localPort);
                    os.append(" -> ");
                    os.append(info.remoteAddress).append(':').append(info.remotePort);
                }
                else
                {
                    os.append("connection-").append(_connectionInfo);
                }
                if(!_connectionInfo.connectionId.isEmpty())
                {
                    os.append(" [").append(_connectionInfo.connectionId).append("]");
                }
                _id = os.toString();
            }
            return _id;
        }

        public String
        getState()
        {
            switch(_state)
            {
            case ConnectionStateValidating:
                return "validating";
            case ConnectionStateHolding:
                return "holding";
            case ConnectionStateActive:
                return "active";
            case ConnectionStateClosing:
                return "closing";
            case ConnectionStateClosed:
                return "closed";
            default:
                assert(false);
                return "";
            }
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

        private com.zeroc.Ice.IPConnectionInfo
        getIPConnectionInfo()
        {
            for(com.zeroc.Ice.ConnectionInfo p = _connectionInfo; p != null; p = p.underlying)
            {
                if(p instanceof com.zeroc.Ice.IPConnectionInfo)
                {
                    return (com.zeroc.Ice.IPConnectionInfo)p;
                }
            }
            return null;
        }

        private final com.zeroc.Ice.ConnectionInfo _connectionInfo;
        private final com.zeroc.Ice.Endpoint _endpoint;
        private final com.zeroc.Ice.Instrumentation.ConnectionState _state;
        private String _id;
        private com.zeroc.Ice.EndpointInfo _endpointInfo;
    }

    static public final class DispatchHelper extends MetricsHelper<DispatchMetrics>
    {
        static private final AttributeResolver _attributes = new AttributeResolver()
        {
            {
                try
                {
                    Class<?> cl = DispatchHelper.class;
                    add("parent", cl.getDeclaredMethod("getParent"));
                    add("id", cl.getDeclaredMethod("getId"));

                    addConnectionAttributes(this, cl);

                    Class<?> clc = com.zeroc.Ice.Current.class;
                    add("operation", cl.getDeclaredMethod("getCurrent"), clc.getDeclaredField("operation"));
                    add("identity", cl.getDeclaredMethod("getIdentity"));
                    add("facet", cl.getDeclaredMethod("getCurrent"), clc.getDeclaredField("facet"));
                    add("requestId", cl.getDeclaredMethod("getCurrent"), clc.getDeclaredField("requestId"));
                    add("mode", cl.getDeclaredMethod("getMode"));
                }
                catch(Exception ex)
                {
                    ex.printStackTrace();
                    assert(false);
                }
            }
        };

        DispatchHelper(com.zeroc.Ice.Current current, int size)
        {
            super(_attributes);
            _current = current;
            _size = size;
        }

        @Override
        public void
        initMetrics(DispatchMetrics v)
        {
            v.size += _size;
        }

        @Override
        protected String
        defaultResolve(String attribute)
        {
            if(attribute.indexOf("context.", 0) == 0)
            {
                String v = _current.ctx.get(attribute.substring(8));
                if(v != null)
                {
                    return v;
                }
            }
            throw new IllegalArgumentException(attribute);
        }

        public String
        getMode()
        {
            return _current.requestId == 0 ? "oneway" : "twoway";
        }

        public String
        getId()
        {
            if(_id == null)
            {
                StringBuilder os = new StringBuilder();
                if(_current.id.category != null && !_current.id.category.isEmpty())
                {
                    os.append(_current.id.category).append('/');
                }
                os.append(_current.id.name).append(" [").append(_current.operation).append(']');
                _id = os.toString();
            }
            return _id;
        }

        public int
        getRequestId()
        {
            return _current.requestId;
        }

        public String
        getParent()
        {
            return _current.adapter.getName();
        }

        public com.zeroc.Ice.ConnectionInfo
        getConnectionInfo()
        {
            if(_current.con != null)
            {
                return _current.con.getInfo();
            }
            return null;
        }

        public com.zeroc.Ice.Endpoint
        getEndpoint()
        {
            if(_current.con != null)
            {
                return _current.con.getEndpoint();
            }
            return null;
        }

        public com.zeroc.Ice.Connection
        getConnection()
        {
            return _current.con;
        }

        public com.zeroc.Ice.EndpointInfo
        getEndpointInfo()
        {
            if(_current.con != null && _endpointInfo == null)
            {
                _endpointInfo = _current.con.getEndpoint().getInfo();
            }
            return _endpointInfo;
        }

        public com.zeroc.Ice.Current
        getCurrent()
        {
            return _current;
        }

        public String
        getIdentity()
        {
            return _current.adapter.getCommunicator().identityToString(_current.id);
        }

        final private com.zeroc.Ice.Current _current;
        final private int _size;
        private String _id;
        private com.zeroc.Ice.EndpointInfo _endpointInfo;
    }

    static public final class InvocationHelper extends MetricsHelper<InvocationMetrics>
    {
        static private final AttributeResolver _attributes = new AttributeResolver()
        {
            {
                try
                {
                    Class<?> cl = InvocationHelper.class;
                    add("parent", cl.getDeclaredMethod("getParent"));
                    add("id", cl.getDeclaredMethod("getId"));

                    add("operation", cl.getDeclaredMethod("getOperation"));
                    add("identity", cl.getDeclaredMethod("getIdentity"));

                    Class<?> cli = com.zeroc.Ice.ObjectPrx.class;
                    add("facet", cl.getDeclaredMethod("getProxy"), cli.getDeclaredMethod("ice_getFacet"));
                    add("encoding", cl.getDeclaredMethod("getEncodingVersion"));
                    add("mode", cl.getDeclaredMethod("getMode"));
                    add("proxy", cl.getDeclaredMethod("getProxy"));
                }
                catch(Exception ex)
                {
                    ex.printStackTrace();
                    assert(false);
                }
            }
        };

        InvocationHelper(com.zeroc.Ice.ObjectPrx proxy, String op, java.util.Map<String, String> ctx)
        {
            super(_attributes);
            _proxy = proxy;
            _operation = op;
            _context = ctx;
        }

        @Override
        protected String
        defaultResolve(String attribute)
        {
            if(attribute.indexOf("context.", 0) == 0)
            {
                String v = _context.get(attribute.substring(8));
                if(v != null)
                {
                    return v;
                }
            }
            throw new IllegalArgumentException(attribute);
        }

        public String
        getMode()
        {
            if(_proxy == null)
            {
                throw new IllegalArgumentException("mode");
            }

            if(_proxy.ice_isTwoway())
            {
                return "twoway";
            }
            else if(_proxy.ice_isOneway())
            {
                return "oneway";
            }
            else if(_proxy.ice_isBatchOneway())
            {
                return "batch-oneway";
            }
            else if(_proxy.ice_isDatagram())
            {
                return "datagram";
            }
            else if(_proxy.ice_isBatchDatagram())
            {
                return "batch-datagram";
            }
            else
            {
                throw new IllegalArgumentException("mode");
            }
        }

        public String
        getId()
        {
            if(_id == null)
            {
                if(_proxy != null)
                {
                    StringBuilder os = new StringBuilder();
                    try
                    {
                        os.append(_proxy.ice_endpoints(emptyEndpoints)).append(" [").append(_operation).append(']');
                    }
                    catch(Exception ex)
                    {
                        // Either a fixed proxy or the communicator is destroyed.
                        os.append(_proxy.ice_getCommunicator().identityToString(_proxy.ice_getIdentity()));
                        os.append(" [").append(_operation).append(']');
                    }
                    _id = os.toString();
                }
                else
                {
                    _id = _operation;
                }
            }
            return _id;
        }

        public String
        getParent()
        {
            return "Communicator";
        }

        public com.zeroc.Ice.ObjectPrx
        getProxy()
        {
            return _proxy;
        }

        public String
        getIdentity()
        {
            if(_proxy != null)
            {
                return _proxy.ice_getCommunicator().identityToString(_proxy.ice_getIdentity());
            }
            else
            {
                return "";
            }
        }

        public String
        getOperation()
        {
            return _operation;
        }

        public String
        getEncodingVersion()
        {
            return com.zeroc.Ice.Util.encodingVersionToString(_proxy.ice_getEncodingVersion());
        }

        final private com.zeroc.Ice.ObjectPrx _proxy;
        final private String _operation;
        final private java.util.Map<String, String> _context;
        private String _id;

        static final private com.zeroc.Ice.Endpoint[] emptyEndpoints = new com.zeroc.Ice.Endpoint[0];
    }

    static public final class ThreadHelper extends MetricsHelper<ThreadMetrics>
    {
        static private final AttributeResolver _attributes = new AttributeResolver()
        {
            {
                try
                {
                    add("parent", ThreadHelper.class.getDeclaredField("_parent"));
                    add("id", ThreadHelper.class.getDeclaredField("_id"));
                }
                catch(Exception ex)
                {
                    assert(false);
                }
            }
        };

        ThreadHelper(String parent, String id, com.zeroc.Ice.Instrumentation.ThreadState state)
        {
            super(_attributes);
            _parent = parent;
            _id = id;
            _state = state;
        }

        @Override
        public void
        initMetrics(ThreadMetrics v)
        {
            switch(_state)
            {
            case ThreadStateInUseForIO:
                ++v.inUseForIO;
                break;
            case ThreadStateInUseForUser:
                ++v.inUseForUser;
                break;
            case ThreadStateInUseForOther:
                ++v.inUseForOther;
                break;
            default:
                break;
            }
        }

        final public String _parent;
        final public String _id;
        final private com.zeroc.Ice.Instrumentation.ThreadState _state;
    }

    static public final class EndpointHelper extends MetricsHelper<Metrics>
    {
        static private final AttributeResolver _attributes = new AttributeResolver()
        {
            {
                try
                {
                    add("parent", EndpointHelper.class.getDeclaredMethod("getParent"));
                    add("id", EndpointHelper.class.getDeclaredMethod("getId"));
                    addEndpointAttributes(this, EndpointHelper.class);
                }
                catch(Exception ex)
                {
                    ex.printStackTrace();
                    assert(false);
                }
            }
        };

        EndpointHelper(com.zeroc.Ice.Endpoint endpt, String id)
        {
            super(_attributes);
            _endpoint = endpt;
            _id = id;
        }

        EndpointHelper(com.zeroc.Ice.Endpoint endpt)
        {
            super(_attributes);
            _endpoint = endpt;
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

        public String
        getParent()
        {
            return "Communicator";
        }

        public String
        getId()
        {
            if(_id == null)
            {
                _id = _endpoint.toString();
            }
            return _id;
        }

        public String
        getEndpoint()
        {
            return _endpoint.toString();
        }

        final private com.zeroc.Ice.Endpoint _endpoint;
        private String _id;
        private com.zeroc.Ice.EndpointInfo _endpointInfo;
    }

    public
    CommunicatorObserverI(com.zeroc.Ice.InitializationData initData)
    {
        _metrics = new MetricsAdminI(initData.properties, initData.logger);
        _delegate = initData.observer;

        _connections = new ObserverFactoryWithDelegate<ConnectionMetrics, ConnectionObserverI,
            com.zeroc.Ice.Instrumentation.ConnectionObserver>(_metrics, "Connection", ConnectionMetrics.class);
        _dispatch = new ObserverFactoryWithDelegate<DispatchMetrics, DispatchObserverI,
            com.zeroc.Ice.Instrumentation.DispatchObserver>(_metrics, "Dispatch", DispatchMetrics.class);
        _invocations = new ObserverFactoryWithDelegate<InvocationMetrics, InvocationObserverI,
            com.zeroc.Ice.Instrumentation.InvocationObserver>(_metrics, "Invocation", InvocationMetrics.class);
        _threads = new ObserverFactoryWithDelegate<ThreadMetrics, ThreadObserverI,
            com.zeroc.Ice.Instrumentation.ThreadObserver>(_metrics, "Thread", ThreadMetrics.class);
        _connects = new ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
            com.zeroc.Ice.Instrumentation.Observer>(_metrics, "ConnectionEstablishment", Metrics.class);
        _endpointLookups = new ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
            com.zeroc.Ice.Instrumentation.Observer>(_metrics, "EndpointLookup", Metrics.class);

        try
        {
            _invocations.registerSubMap("Remote", RemoteMetrics.class,
                                        InvocationMetrics.class.getDeclaredField("remotes"));
            _invocations.registerSubMap("Collocated", CollocatedMetrics.class,
                                        InvocationMetrics.class.getDeclaredField("collocated"));
        }
        catch(Exception ex)
        {
            assert(false);
        }
    }

    @Override
    public com.zeroc.Ice.Instrumentation.Observer
    getConnectionEstablishmentObserver(com.zeroc.Ice.Endpoint endpt, String connector)
    {
        if(_connects.isEnabled())
        {
            try
            {
                com.zeroc.Ice.Instrumentation.Observer delegate = null;
                if(_delegate != null)
                {
                    delegate = _delegate.getConnectionEstablishmentObserver(endpt, connector);
                }
                return _connects.getObserver(new EndpointHelper(endpt, connector), ObserverWithDelegateI.class,
                                             delegate);
            }
            catch(Exception ex)
            {
                _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + Ex.toString(ex));
            }
        }
        return null;
    }

    @Override
    public com.zeroc.Ice.Instrumentation.Observer
    getEndpointLookupObserver(com.zeroc.Ice.Endpoint endpt)
    {
        if(_endpointLookups.isEnabled())
        {
            try
            {
                com.zeroc.Ice.Instrumentation.Observer delegate = null;
                if(_delegate != null)
                {
                    delegate = _delegate.getEndpointLookupObserver(endpt);
                }
                return _endpointLookups.getObserver(new EndpointHelper(endpt), ObserverWithDelegateI.class, delegate);
            }
            catch(Exception ex)
            {
                _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + Ex.toString(ex));
            }

        }
        return null;
    }

    @Override
    public com.zeroc.Ice.Instrumentation.ConnectionObserver
    getConnectionObserver(com.zeroc.Ice.ConnectionInfo c, com.zeroc.Ice.Endpoint e,
                          com.zeroc.Ice.Instrumentation.ConnectionState s,
                          com.zeroc.Ice.Instrumentation.ConnectionObserver observer)
    {
        if(_connections.isEnabled())
        {
            try
            {
                com.zeroc.Ice.Instrumentation.ConnectionObserver delegate = null;
                ConnectionObserverI o = observer instanceof ConnectionObserverI ? (ConnectionObserverI)observer : null;
                if(_delegate != null)
                {
                    delegate = _delegate.getConnectionObserver(c, e, s, o != null ? o.getDelegate() : observer);
                }
                return _connections.getObserver(new ConnectionHelper(c, e, s), o, ConnectionObserverI.class, delegate);
            }
            catch(Exception ex)
            {
                _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + Ex.toString(ex));
            }
        }
        return null;
    }

    @Override
    public com.zeroc.Ice.Instrumentation.ThreadObserver
    getThreadObserver(String parent, String id, com.zeroc.Ice.Instrumentation.ThreadState s,
                      com.zeroc.Ice.Instrumentation.ThreadObserver observer)
    {
        if(_threads.isEnabled())
        {
            try
            {
                com.zeroc.Ice.Instrumentation.ThreadObserver delegate = null;
                ThreadObserverI o = observer instanceof ThreadObserverI ? (ThreadObserverI)observer : null;
                if(_delegate != null)
                {
                    delegate = _delegate.getThreadObserver(parent, id, s, o != null ? o.getDelegate() : observer);
                }
                return _threads.getObserver(new ThreadHelper(parent, id, s), o, ThreadObserverI.class, delegate);
            }
            catch(Exception ex)
            {
                _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + Ex.toString(ex));
            }
        }
        return null;
    }

    @Override
    public com.zeroc.Ice.Instrumentation.InvocationObserver
    getInvocationObserver(com.zeroc.Ice.ObjectPrx prx, String operation, java.util.Map<java.lang.String,
                          java.lang.String> ctx)
    {
        if(_invocations.isEnabled())
        {
            try
            {
                com.zeroc.Ice.Instrumentation.InvocationObserver delegate = null;
                if(_delegate != null)
                {
                    delegate = _delegate.getInvocationObserver(prx, operation, ctx);
                }
                return _invocations.getObserver(new InvocationHelper(prx, operation, ctx),
                                                InvocationObserverI.class,
                                                delegate);
            }
            catch(Exception ex)
            {
                _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + Ex.toString(ex));
            }
        }
        return null;
    }

    @Override
    public com.zeroc.Ice.Instrumentation.DispatchObserver
    getDispatchObserver(com.zeroc.Ice.Current c, int size)
    {
        if(_dispatch.isEnabled())
        {
            try
            {
                com.zeroc.Ice.Instrumentation.DispatchObserver delegate = null;
                if(_delegate != null)
                {
                    delegate = _delegate.getDispatchObserver(c, size);
                }
                return _dispatch.getObserver(new DispatchHelper(c, size), DispatchObserverI.class, delegate);
            }
            catch(Exception ex)
            {
                _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + Ex.toString(ex));
            }
        }
        return null;
    }

    @Override
    public void
    setObserverUpdater(final com.zeroc.Ice.Instrumentation.ObserverUpdater updater)
    {
        if(updater == null)
        {
            _connections.setUpdater(null);
            _threads.setUpdater(null);
        }
        else
        {
            _connections.setUpdater(() -> { updater.updateConnectionObservers(); });
            _threads.setUpdater(() -> { updater.updateThreadObservers(); });
        }

        if(_delegate != null)
        {
            _delegate.setObserverUpdater(updater);
        }
    }

    public MetricsAdminI getFacet()
    {
        return _metrics;
    }

    final private MetricsAdminI _metrics;
    final private com.zeroc.Ice.Instrumentation.CommunicatorObserver _delegate;
    final private ObserverFactoryWithDelegate<ConnectionMetrics, ConnectionObserverI,
        com.zeroc.Ice.Instrumentation.ConnectionObserver> _connections;
    final private ObserverFactoryWithDelegate<DispatchMetrics, DispatchObserverI,
        com.zeroc.Ice.Instrumentation.DispatchObserver> _dispatch;
    final private ObserverFactoryWithDelegate<InvocationMetrics, InvocationObserverI,
        com.zeroc.Ice.Instrumentation.InvocationObserver> _invocations;
    final private ObserverFactoryWithDelegate<ThreadMetrics, ThreadObserverI,
        com.zeroc.Ice.Instrumentation.ThreadObserver> _threads;
    final private ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
        com.zeroc.Ice.Instrumentation.Observer> _connects;
    final private ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
        com.zeroc.Ice.Instrumentation.Observer> _endpointLookups;
}
