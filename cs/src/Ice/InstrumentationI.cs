// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceMX
{
    using System;
    using System.Diagnostics;
    using System.Text;
    using System.Collections.Generic;

    static class Util
    {
        public static void
        addEndpointAttributes<T>(MetricsHelper<T>.AttributeResolver r, Type cl)
        {
            Type cli = typeof(Ice.EndpointInfo);
            r.add("endpointType", cl.GetMethod("getEndpointInfo"), cli.GetMethod("type"));
            r.add("endpointIsDatagram", cl.GetMethod("getEndpointInfo"), cli.GetMethod("datagram"));
            r.add("endpointIsSecure", cl.GetMethod("getEndpointInfo"), cli.GetMethod("secure"));
            r.add("endpointProtocolVersion", cl.GetMethod("getEndpointInfo"), cli.GetField("protocol"));
            r.add("endpointEncodingVersion", cl.GetMethod("getEndpointInfo"), cli.GetField("encoding"));
            r.add("endpointTimeout", cl.GetMethod("getEndpointInfo"), cli.GetField("timeout"));
            r.add("endpointCompress", cl.GetMethod("getEndpointInfo"), cli.GetField("compress"));
            
            cli = typeof(Ice.IPEndpointInfo);
            r.add("endpointHost", cl.GetMethod("getEndpointInfo"), cli.GetField("host"));
            r.add("endpointPort", cl.GetMethod("getEndpointInfo"), cli.GetField("port"));
        }

        public static void
        addConnectionAttributes<T>(MetricsHelper<T>.AttributeResolver r, Type cl)
        {
            Type cli = typeof(Ice.ConnectionInfo);
            r.add("incoming", cl.GetMethod("getConnectionInfo"), cli.GetField("incoming"));
            r.add("adapterName", cl.GetMethod("getConnectionInfo"), cli.GetField("adapterName"));
            r.add("connectionId", cl.GetMethod("getConnectionInfo"), cli.GetField("connectionId"));
                
            cli = typeof(Ice.IPConnectionInfo);
            r.add("localHost", cl.GetMethod("getConnectionInfo"), cli.GetField("localAddress"));
            r.add("localPort", cl.GetMethod("getConnectionInfo"), cli.GetField("localPort"));
            r.add("remoteHost", cl.GetMethod("getConnectionInfo"), cli.GetField("remoteAddress"));
            r.add("remotePort", cl.GetMethod("getConnectionInfo"), cli.GetField("remotePort"));
            
            cli = typeof(Ice.UDPConnectionInfo);
            r.add("mcastHost", cl.GetMethod("getConnectionInfo"), cli.GetField("mcastAddress"));
            r.add("mcastPort", cl.GetMethod("getConnectionInfo"), cli.GetField("mcastPort"));
            
            Util.addEndpointAttributes<T>(r, cl);
        }
    }

    class ConnectionHelper : MetricsHelper<ConnectionMetrics>
    {
        class AttributeResolverI : MetricsHelper<ConnectionMetrics>.AttributeResolver
        {
            public AttributeResolverI()
            {
                try
                {
                    Type cl = typeof(ConnectionHelper);
                    add("parent", cl.GetMethod("getParent"));
                    add("id", cl.GetMethod("getId"));
                    add("endpoint", cl.GetMethod("getEndpoint"));
                    add("state", cl.GetMethod("getState"));
                    Util.addConnectionAttributes<ConnectionMetrics>(this, cl);
                }
                catch(Exception)
                {
                    Debug.Assert(false);
                }
            }
        };
        static AttributeResolver _attributes = new AttributeResolverI();
        
        public ConnectionHelper(Ice.ConnectionInfo con, Ice.Endpoint endpt, Ice.Instrumentation.ConnectionState state) 
            : base(_attributes)
        {
            _connectionInfo = con;
            _endpoint = endpt;
            _state = state;
        }

        public string getId()
        {
            if(_id == null)
            {
                StringBuilder os = new StringBuilder();
                if(_connectionInfo is Ice.IPConnectionInfo)
                {
                    Ice.IPConnectionInfo info = (Ice.IPConnectionInfo)_connectionInfo;
                    os.Append(info.localAddress).Append(':').Append(info.localPort);
                    os.Append(" -> ");
                    os.Append(info.remoteAddress).Append(':').Append(info.remotePort);
                }
                else
                {
                    os.Append("connection-").Append(_connectionInfo);
                }
                if(_connectionInfo.connectionId.Length > 0)
                {
                    os.Append(" [").Append(_connectionInfo.connectionId).Append("]");
                }
                _id = os.ToString();
            }
            return _id;
        }

        public string getState()
        {
            switch(_state)
            {
            case Ice.Instrumentation.ConnectionState.ConnectionStateValidating:
                return "validating";
            case Ice.Instrumentation.ConnectionState.ConnectionStateHolding:
                return "holding";
            case Ice.Instrumentation.ConnectionState.ConnectionStateActive:
                return "active";
            case Ice.Instrumentation.ConnectionState.ConnectionStateClosing:
                return "closing";
            case Ice.Instrumentation.ConnectionState.ConnectionStateClosed:
                return "closed";
            default:
                Debug.Assert(false);
                return "";
            }
        }
        
        public string getParent()
        {
            if(_connectionInfo.adapterName != null && _connectionInfo.adapterName.Length > 0)
            {
                return _connectionInfo.adapterName;
            }
            else
            {
                return "Communicator";
            }
        }
    
        public Ice.ConnectionInfo getConnectionInfo()
        {
            return _connectionInfo;
        }

        public Ice.Endpoint getEndpoint()
        {
            return _endpoint;
        }

        public Ice.EndpointInfo getEndpointInfo()
        {
            if(_endpointInfo == null)
            {
                _endpointInfo = _endpoint.getInfo();
            }
            return _endpointInfo;
        }
    
        readonly private Ice.ConnectionInfo _connectionInfo;
        readonly private Ice.Endpoint _endpoint;
        readonly private Ice.Instrumentation.ConnectionState _state;
        private string _id;
        private Ice.EndpointInfo _endpointInfo;
    };

    class DispatchHelper : MetricsHelper<Metrics>
    {
        class AttributeResolverI : MetricsHelper<Metrics>.AttributeResolver
        {
            public AttributeResolverI()
            {
                try
                {
                    Type cl = typeof(DispatchHelper);
                    add("parent", cl.GetMethod("getParent"));
                    add("id", cl.GetMethod("getId"));
                    add("endpoint", cl.GetMethod("getEndpoint"));
                    add("connection", cl.GetMethod("getConnection"));
                    
                    Util.addConnectionAttributes<Metrics>(this, cl);
                    
                    Type clc = typeof(Ice.Current);
                    add("operation", cl.GetMethod("getCurrent"), clc.GetField("operation"));
                    add("identity", cl.GetMethod("getIdentity"));
                    add("facet", cl.GetMethod("getCurrent"), clc.GetField("facet"));
                    add("mode", cl.GetMethod("getMode"));
                }
                catch(Exception)
                {
                    Debug.Assert(false);
                }
            }
        };
        static AttributeResolver _attributes = new AttributeResolverI();
        
        public DispatchHelper(Ice.Current current) : base(_attributes)
        {
            _current = current;
        }

        override protected string defaultResolve(string attribute)
        {
            if(attribute.IndexOf("context.", 0) == 0)
            {
                string v;
                if(_current.ctx.TryGetValue(attribute.Substring(8), out v))
                {
                    return v;
                }
            }
            throw new ArgumentOutOfRangeException(attribute);
        }
        
        public string getMode()
        {
            return _current.requestId == 0 ? "oneway" : "twoway";
        }
        
        public string getId()
        {
            if(_id == null)
            {
                StringBuilder os = new StringBuilder();
                if(_current.id.category != null && _current.id.category.Length > 0)
                {
                    os.Append(_current.id.category).Append('/');
                }
                os.Append(_current.id.name).Append(" [").Append(_current.operation).Append(']');
                _id = os.ToString();
            }
            return _id;
        }

        public string getParent()
        {
            return _current.adapter.getName();
        }
        
        public Ice.ConnectionInfo getConnectionInfo()
        {
            return _current.con.getInfo();
        }
        
        public Ice.Endpoint getEndpoint()
        {
            return _current.con.getEndpoint();
        }

        public Ice.Connection getConnection()
        {
            return _current.con;
        }
        
        public Ice.EndpointInfo getEndpointInfo()
        {
            if(_endpointInfo == null)
            {
                _endpointInfo = _current.con.getEndpoint().getInfo();
            }
            return _endpointInfo;
        }

        public Ice.Current getCurrent()
        {
            return _current;
        }
        
        public string getIdentity()
        {
            return _current.adapter.getCommunicator().identityToString(_current.id);
        }
        
        readonly private Ice.Current _current;
        private string _id;
        private Ice.EndpointInfo _endpointInfo;
    };

    class InvocationHelper : MetricsHelper<InvocationMetrics>
    {
        class AttributeResolverI : MetricsHelper<InvocationMetrics>.AttributeResolver
        { 
            public AttributeResolverI()
            {
                try
                {
                    Type cl = typeof(InvocationHelper);
                    add("parent", cl.GetMethod("getParent"));
                    add("id", cl.GetMethod("getId"));
                        
                    add("operation", cl.GetMethod("getOperation"));
                    add("identity", cl.GetMethod("getIdentity"));
                        
                    Type cli = typeof(Ice.ObjectPrx);
                    add("facet", cl.GetMethod("getProxy"), cli.GetMethod("ice_getFacet"));
                    add("encoding", cl.GetMethod("getProxy"), cli.GetMethod("ice_getEncodingVersion"));
                    add("mode", cl.GetMethod("getMode"));
                    add("proxy", cl.GetMethod("getProxy"));
                }
                catch(Exception)
                {
                    Debug.Assert(false);
                }
            }
        };
        static AttributeResolver _attributes = new AttributeResolverI();
        
        public InvocationHelper(Ice.ObjectPrx proxy, string op, Dictionary<string, string> ctx) : base(_attributes)
        {
            _proxy = proxy;
            _operation = op;
            _context = ctx;
        }

        override protected string defaultResolve(string attribute)
        {
            if(attribute.IndexOf("context.", 0) == 0)
            {
                string v;
                if(_context.TryGetValue(attribute.Substring(8), out v))
                {
                    return v;
                }
            }
            throw new ArgumentOutOfRangeException(attribute);
        }
        
        public string getMode()
        {
            if(_proxy == null)
            {
                throw new ArgumentOutOfRangeException("mode");
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
                throw new ArgumentOutOfRangeException("mode");
            }
        }

        public string getId()
        {
            if(_id == null)
            {
                if(_proxy != null)
                {
                    StringBuilder os = new StringBuilder();
                    try
                    {
                        os.Append(_proxy).Append(" [").Append(_operation).Append(']');
                    }
                    catch(Ice.FixedProxyException)
                    {
                        os.Append(_proxy.ice_getCommunicator().identityToString(_proxy.ice_getIdentity()));
                        os.Append(" [").Append(_operation).Append(']');
                    }
                    _id = os.ToString();
                }
                else
                {
                    _id = _operation;
                }
            }
            return _id;
        }
        
        public string getParent()
        {
            return "Communicator";
        }
        
        public Ice.ObjectPrx getProxy()
        {
            return _proxy;
        }
        
        public string getIdentity()
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
        
        public string getOperation()
        {
            return _operation;
        }
        
        readonly private Ice.ObjectPrx _proxy;
        readonly private string _operation;
        readonly private Dictionary<string, string> _context;
        private string _id;
    };
    
    class ThreadHelper : MetricsHelper<ThreadMetrics>
    {
        class AttributeResolverI : MetricsHelper<ThreadMetrics>.AttributeResolver
        { 
            public AttributeResolverI()
            {
                try
                {
                    Type cl = typeof(ThreadHelper);
                    add("parent", cl.GetField("_parent"));
                    add("id", cl.GetField("_id"));
                }
                catch(Exception)
                {
                    Debug.Assert(false);
                }
            }
        };
        static AttributeResolver _attributes = new AttributeResolverI();

        public ThreadHelper(string parent, string id, Ice.Instrumentation.ThreadState state) : base(_attributes)
        {
            _parent = parent;
            _id = id;
            _state = state;
        }

        override public void initMetrics(ThreadMetrics v)
        {
            switch(_state)
            {
            case Ice.Instrumentation.ThreadState.ThreadStateInUseForIO:
                ++v.inUseForIO;
                break;
            case Ice.Instrumentation.ThreadState.ThreadStateInUseForUser:
                ++v.inUseForUser;
                break;
            case Ice.Instrumentation.ThreadState.ThreadStateInUseForOther:
                ++v.inUseForOther;
                break;
            default:
                break;
            }
        }
        
        readonly public string _parent;
        readonly public string _id;
        readonly private Ice.Instrumentation.ThreadState _state;
    };

    class EndpointHelper : MetricsHelper<Metrics>
    {
        class AttributeResolverI : MetricsHelper<Metrics>.AttributeResolver
        { 
            public AttributeResolverI()
            {
                try
                {
                    Type cl = typeof(EndpointHelper);
                    add("parent", cl.GetMethod("getParent"));
                    add("id", cl.GetMethod("getId"));
                    add("endpoint", cl.GetMethod("getEndpoint"));
                    Util.addEndpointAttributes<Metrics>(this, cl);
                }
                catch(Exception)
                {
                    Debug.Assert(false);
                }
            }
        };
        static AttributeResolver _attributes = new AttributeResolverI();

        public EndpointHelper(Ice.Endpoint endpt, string id) : base(_attributes)
        {
            _endpoint = endpt;
            _id = id;
        }

        public EndpointHelper(Ice.Endpoint endpt) : base(_attributes)
        {
            _endpoint = endpt;
        }
        
        public Ice.EndpointInfo getEndpointInfo()
        {
            if(_endpointInfo == null)
            {
                _endpointInfo = _endpoint.getInfo();
            }
            return _endpointInfo;
        }
        
        public string getParent()
        {
            return "Communicator";
        }
        
        public string getId()
        {
            if(_id == null)
            {
                _id = _endpoint.ToString();
            }
            return _id;
        }
        
        public string getEndpoint()
        {
            return _endpoint.ToString();
        }
        
        readonly private Ice.Endpoint _endpoint;
        private string _id;
        private Ice.EndpointInfo _endpointInfo;
    };
    
    public class RemoteInvocationHelper : MetricsHelper<Metrics>
    {
        class AttributeResolverI : MetricsHelper<Metrics>.AttributeResolver
        { 
            public AttributeResolverI()
            {
                try
                {
                    Type cl = typeof(RemoteInvocationHelper);
                    add("parent", cl.GetMethod("getParent"));
                    add("id", cl.GetMethod("getId"));
                    add("endpoint", cl.GetMethod("getEndpoint"));
                    Util.addConnectionAttributes<Metrics>(this, cl);
                }
                catch(Exception)
                {
                    Debug.Assert(false);
                }
            }
        };
        static AttributeResolver _attributes = new AttributeResolverI();

        public RemoteInvocationHelper(Ice.ConnectionInfo con, Ice.Endpoint endpt) : base(_attributes)
        {
            _connectionInfo = con;
            _endpoint = endpt;
        }

        public string getId()
        {
            if(_id == null)
            {
                _id = _endpoint.ToString();
                if(_connectionInfo.connectionId != null && _connectionInfo.connectionId.Length > 0)
                {
                    _id += " [" + _connectionInfo.connectionId + "]";
                }
            }
            return _id;
        }
    
        public string getParent()
        {
            if(_connectionInfo.adapterName != null && _connectionInfo.adapterName.Length > 0)
            {
                return _connectionInfo.adapterName;
            }
            else
            {
                return "Communicator";
            }
        }
        
        public Ice.ConnectionInfo getConnectionInfo()
        {
            return _connectionInfo;
        }
        
        public Ice.Endpoint getEndpoint()
        {
            return _endpoint;
        }

        public Ice.EndpointInfo getEndpointInfo()
        {
            if(_endpointInfo == null)
            {
                _endpointInfo = _endpoint.getInfo();
            }
            return _endpointInfo;
        }
    
        readonly private Ice.ConnectionInfo _connectionInfo;
        readonly private Ice.Endpoint _endpoint;
        private string _id;
        private Ice.EndpointInfo _endpointInfo;
    };

    public class ObserverI :Observer<Metrics>
    {
    };

    public class ConnectionObserverI : Observer<ConnectionMetrics>, Ice.Instrumentation.ConnectionObserver
    {
        public void sentBytes(int num)
        {
            _sentBytes = num;
            forEach(sentBytesUpdate);
        }

        public void receivedBytes(int num)
        {
            _receivedBytes = num;
            forEach(receivedBytesUpdate);
        }

        private void sentBytesUpdate(ConnectionMetrics v)
        {
            v.sentBytes += _sentBytes;
        }

        private void receivedBytesUpdate(ConnectionMetrics v)
        {
            v.receivedBytes += _receivedBytes;
        }

        private int _sentBytes;
        private int _receivedBytes;
    };

    public class InvocationObserverI : Observer<InvocationMetrics>, Ice.Instrumentation.InvocationObserver
    {
        public void
        retried()
        {
            forEach(incrementRetry);
        }
    
        public Ice.Instrumentation.Observer getRemoteObserver(Ice.ConnectionInfo con, Ice.Endpoint endpt)
        {
            return getObserver<Metrics, ObserverI>("Remote", new RemoteInvocationHelper(con, endpt));
        }

        private void incrementRetry(InvocationMetrics v)
        {
            ++v.retry;
        }
    }

    public class ThreadObserverI : Observer<ThreadMetrics>, Ice.Instrumentation.ThreadObserver
    {
        public void stateChanged(Ice.Instrumentation.ThreadState oldState, Ice.Instrumentation.ThreadState newState)
        {
            _oldState = oldState;
            _newState = newState;
            forEach(threadStateUpdate);
        }

        private void threadStateUpdate(ThreadMetrics v)
        {
            switch(_oldState)
            {
            case Ice.Instrumentation.ThreadState.ThreadStateInUseForIO:
                --v.inUseForIO;
                break;
            case Ice.Instrumentation.ThreadState.ThreadStateInUseForUser:
                --v.inUseForUser;
                break;
            case Ice.Instrumentation.ThreadState.ThreadStateInUseForOther:
                --v.inUseForOther;
                break;
            default:
                break;
            }
            switch(_newState)
            {
            case Ice.Instrumentation.ThreadState.ThreadStateInUseForIO:
                ++v.inUseForIO;
                break;
            case Ice.Instrumentation.ThreadState.ThreadStateInUseForUser:
                ++v.inUseForUser;
                break;
            case Ice.Instrumentation.ThreadState.ThreadStateInUseForOther:
                ++v.inUseForOther;
                break;
            default:
                break;
            }
        }

        private Ice.Instrumentation.ThreadState _oldState;
        private Ice.Instrumentation.ThreadState _newState;
    };

    public class CommunicatorObserverI : Ice.Instrumentation.CommunicatorObserver
    {
        public CommunicatorObserverI(IceInternal.MetricsAdminI metrics)
        {
            _metrics = metrics;
            
            _connections = new ObserverFactory<ConnectionMetrics, ConnectionObserverI>(metrics, "Connection");
            _dispatch = new ObserverFactory<Metrics, ObserverI>(metrics, "Dispatch");
            _invocations = new ObserverFactory<InvocationMetrics, InvocationObserverI>(metrics, "Invocation");
            _threads = new ObserverFactory<ThreadMetrics, ThreadObserverI>(metrics, "Thread");
            _connects = new ObserverFactory<Metrics, ObserverI>(metrics, "ConnectionEstablishment");
            _endpointLookups = new ObserverFactory<Metrics, ObserverI>(metrics, "EndpointLookup");

            try
            {
                _invocations.registerSubMap<Metrics>("Remote", typeof(InvocationMetrics).GetField("remotes"));
            }
            catch(Exception)
            {
                Debug.Assert(false);
            }
        }

        public Ice.Instrumentation.Observer getConnectionEstablishmentObserver(Ice.Endpoint endpt, string connector)
        {
            if(_connects.isEnabled())
            {
                return _connects.getObserver(new EndpointHelper(endpt, connector));
            }
            return null;
        }

        public Ice.Instrumentation.Observer getEndpointLookupObserver(Ice.Endpoint endpt)
        {
            if(_endpointLookups.isEnabled())
            {
                return _endpointLookups.getObserver(new EndpointHelper(endpt));
            }
            return null;
        }
    
        public Ice.Instrumentation.ConnectionObserver getConnectionObserver(Ice.ConnectionInfo c, Ice.Endpoint e,
                                                                            Ice.Instrumentation.ConnectionState s,
                                                                            Ice.Instrumentation.ConnectionObserver o)
        {
            if(_connections.isEnabled())
            {
                return _connections.getObserver(new ConnectionHelper(c, e, s), o);
            }
            return null;
        }
    
        public Ice.Instrumentation.ThreadObserver getThreadObserver(string parent, string id, 
                                                                    Ice.Instrumentation.ThreadState s, 
                                                                    Ice.Instrumentation.ThreadObserver o)
        {
            if(_threads.isEnabled())
            {
                return _threads.getObserver(new ThreadHelper(parent, id, s), o);
            }
            return null;
        }
    
        public Ice.Instrumentation.InvocationObserver getInvocationObserver(Ice.ObjectPrx prx, string operation, 
                                                                            Dictionary<string, string> ctx)
        {
            if(_invocations.isEnabled())
            {
                return _invocations.getObserver(new InvocationHelper(prx, operation, ctx));
            }
            return null;
        }
    
        public Ice.Instrumentation.Observer getDispatchObserver(Ice.Current c)
        {
            if(_dispatch.isEnabled())
            {
                return _dispatch.getObserver(new DispatchHelper(c));
            }
            return null;
        }
    
        public void setObserverUpdater(Ice.Instrumentation.ObserverUpdater updater)
        {
            _connections.setUpdater(updater.updateConnectionObservers);
            _threads.setUpdater(updater.updateThreadObservers);
        }

        readonly private IceInternal.MetricsAdminI _metrics;
        readonly private ObserverFactory<ConnectionMetrics, ConnectionObserverI> _connections;
        readonly private ObserverFactory<Metrics, ObserverI> _dispatch;
        readonly private ObserverFactory<InvocationMetrics, InvocationObserverI> _invocations;
        readonly private ObserverFactory<ThreadMetrics, ThreadObserverI> _threads;
        readonly private ObserverFactory<Metrics, ObserverI> _connects;
        readonly private ObserverFactory<Metrics, ObserverI> _endpointLookups;
    }
}