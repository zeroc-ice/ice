// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceMX;

public class CommunicatorObserverI implements Ice.Instrumentation.CommunicatorObserver
{
    static void
    addEndpointAttributes(MetricsHelper.AttributeResolver r, Class<?> cl)
        throws Exception
    {
        Class<?> cli = Ice.EndpointInfo.class;
        r.add("endpointType", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredMethod("type"));
        r.add("endpointIsDatagram", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredMethod("datagram"));
        r.add("endpointIsSecure", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredMethod("secure"));
        r.add("endpointProtocolVersion", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredField("protocol"));
        r.add("endpointEncodingVersion", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredField("encoding"));
        r.add("endpointTimeout", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredField("timeout"));
        r.add("endpointCompress", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredField("compress"));
        
        cli = Ice.IPEndpointInfo.class;
        r.add("endpointHost", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredField("host"));
        r.add("endpointPort", cl.getDeclaredMethod("getEndpointInfo"), cli.getDeclaredField("port"));
    }

    static void
    addConnectionAttributes(MetricsHelper.AttributeResolver r, Class<?> cl)
        throws Exception
    {
        Class<?> cli = Ice.ConnectionInfo.class;
        r.add("incoming", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("incoming"));
        r.add("adapterName", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("adapterName"));
        r.add("connectionId", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("connectionId"));
        
        cli = Ice.IPConnectionInfo.class;
        r.add("localHost", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("localAddress"));
        r.add("localPort", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("localPort"));
        r.add("remoteHost", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("remoteAddress"));
        r.add("remotePort", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("remotePort"));
        
        cli = Ice.UDPConnectionInfo.class;
        r.add("mcastHost", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("mcastAddress"));
        r.add("mcastPort", cl.getDeclaredMethod("getConnectionInfo"), cli.getDeclaredField("mcastPort"));
        
        addEndpointAttributes(r, cl);
    }

    static private class ConnectionHelper extends MetricsHelper<ConnectionMetrics>
    {
        static private AttributeResolver _attributes = new AttributeResolver()
            {
                {
                    try
                    {
                        add("parent", ConnectionHelper.class.getDeclaredMethod("getParent"));
                        add("id", ConnectionHelper.class.getDeclaredMethod("getId"));
                        add("endpoint", ConnectionHelper.class.getDeclaredMethod("getEndpoint"));
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

        ConnectionHelper(Ice.ConnectionInfo con, Ice.Endpoint endpt, Ice.Instrumentation.ConnectionState state)
        {
            super(_attributes);
            _connectionInfo = con;
            _endpoint = endpt;
            _state = state;
        }

        String
        getId()
        {
            if(_id == null)
            {
                StringBuilder os = new StringBuilder();
                if(_connectionInfo instanceof Ice.IPConnectionInfo)
                {
                    Ice.IPConnectionInfo info = (Ice.IPConnectionInfo)_connectionInfo;
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

        String
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
    
        String 
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
    
        Ice.ConnectionInfo
        getConnectionInfo()
        {
            return _connectionInfo;
        }

        Ice.Endpoint
        getEndpoint()
        {
            return _endpoint;
        }

        Ice.EndpointInfo
        getEndpointInfo()
        {
            if(_endpointInfo == null)
            {
                _endpointInfo = _endpoint.getInfo();
            }
            return _endpointInfo;
        }
    
        private final Ice.ConnectionInfo _connectionInfo;
        private final Ice.Endpoint _endpoint;
        private final Ice.Instrumentation.ConnectionState _state;
        private String _id;
        private Ice.EndpointInfo _endpointInfo;
    };

    static private final class DispatchHelper extends MetricsHelper<Metrics>
    {
        static private final AttributeResolver _attributes = new AttributeResolver()
            {
                {
                    try
                    {
                        Class<?> cl = DispatchHelper.class;
                        add("parent", cl.getDeclaredMethod("getParent"));
                        add("id", cl.getDeclaredMethod("getId"));
                        add("endpoint", cl.getDeclaredMethod("getEndpoint"));
                        add("connection", cl.getDeclaredMethod("getConnection"));
                        
                        addConnectionAttributes(this, cl);
                        
                        Class<?> clc = Ice.Current.class;
                        add("operation", cl.getDeclaredMethod("getCurrent"), clc.getDeclaredField("operation"));
                        add("identity", cl.getDeclaredMethod("getIdentity"));
                        add("facet", cl.getDeclaredMethod("getCurrent"), clc.getDeclaredField("facet"));
                        add("mode", cl.getDeclaredMethod("getMode"));
                    }
                    catch(Exception ex)
                    {
                        ex.printStackTrace();
                        assert(false);
                    }
                }
            };
        
        DispatchHelper(Ice.Current current)
        {
            super(_attributes);
            _current = current;
        }

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
        
        String
        getMode()
        {
            return _current.requestId == 0 ? "oneway" : "twoway";
        }
        
        String
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

        String 
        getParent()
        {
            return _current.adapter.getName();
        }
        
        Ice.ConnectionInfo
        getConnectionInfo()
        {
            return _current.con.getInfo();
            }
        
        Ice.Endpoint
        getEndpoint()
        {
            return _current.con.getEndpoint();
        }

        Ice.Connection
        getConnection()
        {
            return _current.con;
        }
        
        Ice.EndpointInfo
        getEndpointInfo()
        {
            if(_endpointInfo == null)
            {
                _endpointInfo = _current.con.getEndpoint().getInfo();
            }
            return _endpointInfo;
        }

        Ice.Current
        getCurrent()
        {
            return _current;
        }
        
        String
        getIdentity()
        {
            return _current.adapter.getCommunicator().identityToString(_current.id);
        }
        
        final private Ice.Current _current;
        private String _id;
        private Ice.EndpointInfo _endpointInfo;
    };

    static private final class InvocationHelper extends MetricsHelper<InvocationMetrics>
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
                        
                        Class<?> cli = Ice.ObjectPrx.class;
                        add("facet", cl.getDeclaredMethod("getProxy"), cli.getDeclaredMethod("ice_getFacet"));
                        add("encoding", cl.getDeclaredMethod("getProxy"),
                            cli.getDeclaredMethod("ice_getEncodingVersion"));
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
        
        InvocationHelper(Ice.ObjectPrx proxy, String op, java.util.Map<String, String> ctx)
        {
            super(_attributes);
            _proxy = proxy;
            _operation = op;
            _context = ctx;
        }

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
        
        String
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

        String
        getId()
        {
            if(_id == null)
            {
                if(_proxy != null)
                {
                    StringBuilder os = new StringBuilder();
                    try
                    {
                        os.append(_proxy).append(" [").append(_operation).append(']');
                    }
                    catch(Ice.FixedProxyException ex)
                    {
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
        
        String 
        getParent()
        {
            return "Communicator";
        }
        
        Ice.ObjectPrx
        getProxy()
        {
            return _proxy;
        }
        

        String
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

        String
        getOperation()
        {
            return _operation;
        }
        
        final private Ice.ObjectPrx _proxy;
        final private String _operation;
        final private java.util.Map<String, String> _context;
        private String _id;
    };
    
    static private final class ThreadHelper extends MetricsHelper<ThreadMetrics>
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

        ThreadHelper(String parent, String id, Ice.Instrumentation.ThreadState state)
        {
            super(_attributes);
            _parent = parent;
            _id = id;
            _state = state;
        }

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
        
        final String _parent;
        final String _id;
        final private Ice.Instrumentation.ThreadState _state;
    };

    static private final class EndpointHelper extends MetricsHelper<Metrics>
    {
        static private final AttributeResolver _attributes = new AttributeResolver()
            { 
                {
                    try
                    {
                        add("parent", EndpointHelper.class.getDeclaredMethod("getParent"));
                        add("id", EndpointHelper.class.getDeclaredMethod("getId"));
                        add("endpoint", EndpointHelper.class.getDeclaredMethod("getEndpoint"));
                        addEndpointAttributes(this, EndpointHelper.class);
                    }
                    catch(Exception ex)
                    {
                        ex.printStackTrace();
                        assert(false);
                    }
                }
            };

        EndpointHelper(Ice.Endpoint endpt, String id)
        {
            super(_attributes);
            _endpoint = endpt;
            _id = id;
        }

        EndpointHelper(Ice.Endpoint endpt)
        {
            super(_attributes);
            _endpoint = endpt;
        }
        
        Ice.EndpointInfo
        getEndpointInfo()
        {
            if(_endpointInfo == null)
            {
                _endpointInfo = _endpoint.getInfo();
            }
            return _endpointInfo;
        }
        
        String
        getParent()
        {
            return "Communicator";
        }
        
        String
        getId()
        {
            if(_id == null)
            {
                _id = _endpoint.toString();
            }
            return _id;
        }
        
        String
        getEndpoint()
        {
            return _endpoint.toString();
        }
        
        final private Ice.Endpoint _endpoint;
        private String _id;
        private Ice.EndpointInfo _endpointInfo;
    };

    public 
    CommunicatorObserverI(IceInternal.MetricsAdminI metrics)
    {
        _metrics = metrics;

        _connections = new ObserverFactory<ConnectionMetrics, ConnectionObserverI>(metrics, "Connection", 
                                                                                   ConnectionMetrics.class);
        _dispatch = new ObserverFactory<Metrics, ObserverI>(metrics, "Dispatch", Metrics.class);
        _invocations = new ObserverFactory<InvocationMetrics, InvocationObserverI>(metrics, "Invocation", 
                                                                                   InvocationMetrics.class);
        _threads = new ObserverFactory<ThreadMetrics, ThreadObserverI>(metrics, "Thread", ThreadMetrics.class);
        _connects = new ObserverFactory<Metrics, ObserverI>(metrics, "ConnectionEstablishment", Metrics.class);
        _endpointLookups = new ObserverFactory<Metrics, ObserverI>(metrics, "EndpointLookup", Metrics.class);

        try
        {
            _invocations.registerSubMap("Remote", Metrics.class, InvocationMetrics.class.getDeclaredField("remotes"));
        }
        catch(Exception ex)
        {
            assert(false);
        }
    }

    public Ice.Instrumentation.Observer
    getConnectionEstablishmentObserver(Ice.Endpoint endpt, String connector)
    {
        if(_connects.isEnabled())
        {
            try
            {
                return _connects.getObserver(new EndpointHelper(endpt, connector), ObserverI.class);
            }
            catch(Exception ex)
            {
                _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + ex);
            }
        }
        return null;
    }

    public Ice.Instrumentation.Observer 
    getEndpointLookupObserver(Ice.Endpoint endpt)
    {
        if(_endpointLookups.isEnabled())
        {
            try
            {
                return _endpointLookups.getObserver(new EndpointHelper(endpt), ObserverI.class);
            }
            catch(Exception ex)
            {
                _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + ex);
            }

        }
        return null;
    }
    
    public Ice.Instrumentation.ConnectionObserver 
    getConnectionObserver(Ice.ConnectionInfo c, Ice.Endpoint e, Ice.Instrumentation.ConnectionState s,
                          Ice.Instrumentation.ConnectionObserver o)
    {
        if(_connections.isEnabled())
        {
            try
            {
                return _connections.getObserver(new ConnectionHelper(c, e, s), o, ConnectionObserverI.class);
            }
            catch(Exception ex)
            {
                _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + ex);
            }
        }
        return null;
    }
    
    public Ice.Instrumentation.ThreadObserver 
    getThreadObserver(String parent, String id, Ice.Instrumentation.ThreadState s, Ice.Instrumentation.ThreadObserver o)
    {
        if(_threads.isEnabled())
        {
            try
            {
                return _threads.getObserver(new ThreadHelper(parent, id, s), o, ThreadObserverI.class);
            }
            catch(Exception ex)
            {
                _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + ex);
            }
        }
        return null;
    }
    
    public Ice.Instrumentation.InvocationObserver
    getInvocationObserver(Ice.ObjectPrx prx, String operation, java.util.Map<java.lang.String, java.lang.String> ctx)
    {
        if(_invocations.isEnabled())
        {
            try
            {
                return _invocations.getObserver(new InvocationHelper(prx, operation, ctx), InvocationObserverI.class);
            }
            catch(Exception ex)
            {
                _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + ex);
            }
        }
        return null;
    }
    
    public Ice.Instrumentation.Observer
    getDispatchObserver(Ice.Current c)
    {
        if(_dispatch.isEnabled())
        {
            try
            {
                return _dispatch.getObserver(new DispatchHelper(c), ObserverI.class);
            }
            catch(Exception ex)
            {
                _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + ex);
            }
        }
        return null;
    }
    
    public void 
    setObserverUpdater(final Ice.Instrumentation.ObserverUpdater updater)
    {
        _connections.setUpdater(new Runnable() {
                public void 
                run()
                {
                    updater.updateConnectionObservers();
                }
            });
        _threads.setUpdater(new Runnable() { 
                public void
                run()
                {
                    updater.updateThreadObservers();
                }
            });
    }

    public IceInternal.MetricsAdminI getMetricsAdmin()
    {
        return _metrics;
    }

    final private IceInternal.MetricsAdminI _metrics;
    final private ObserverFactory<ConnectionMetrics, ConnectionObserverI> _connections;
    final private ObserverFactory<Metrics, ObserverI> _dispatch;
    final private ObserverFactory<InvocationMetrics, InvocationObserverI> _invocations;
    final private ObserverFactory<ThreadMetrics, ThreadObserverI> _threads;
    final private ObserverFactory<Metrics, ObserverI> _connects;
    final private ObserverFactory<Metrics, ObserverI> _endpointLookups;
}
