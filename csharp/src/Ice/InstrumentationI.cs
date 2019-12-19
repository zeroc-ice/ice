//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Text;
    using IceMX;
    using Ice;

    public class ObserverWithDelegate<T, O> : Observer<T>
        where T : Metrics, new()
        where O : Ice.Instrumentation.Observer
    {
        public override void
        attach()
        {
            base.attach();
            if (delegate_ != null)
            {
                delegate_.attach();
            }
        }

        public override void
        detach()
        {
            base.detach();
            if (delegate_ != null)
            {
                delegate_.detach();
            }
        }

        public override void
        failed(string exceptionName)
        {
            base.failed(exceptionName);
            if (delegate_ != null)
            {
                delegate_.failed(exceptionName);
            }
        }

        public O
        getDelegate()
        {
            return delegate_;
        }

        public void
        setDelegate(O del)
        {
            delegate_ = del;
        }

        public Observer getObserver<S, ObserverImpl, Observer>(string mapName, MetricsHelper<S> helper, Observer del)
            where S : Metrics, new()
            where ObserverImpl : ObserverWithDelegate<S, Observer>, Observer, new()
            where Observer : Ice.Instrumentation.Observer
        {
            ObserverImpl obsv = getObserver<S, ObserverImpl>(mapName, helper);
            if (obsv != null)
            {
                obsv.setDelegate(del);
                return obsv;
            }
            return del;
        }

        protected O delegate_;
    }

    public class ObserverFactoryWithDelegate<T, OImpl, O> : ObserverFactory<T, OImpl>
        where T : Metrics, new()
        where OImpl : ObserverWithDelegate<T, O>, O, new()
        where O : Ice.Instrumentation.Observer
    {
        public ObserverFactoryWithDelegate(MetricsAdminI metrics, string name) : base(metrics, name)
        {
        }

        public O getObserver(MetricsHelper<T> helper, O del)
        {
            OImpl o = getObserver(helper);
            if (o != null)
            {
                o.setDelegate(del);
                return o;
            }
            return del;
        }

        public O getObserver(MetricsHelper<T> helper, object observer, O del)
        {
            OImpl o = getObserver(helper, observer);
            if (o != null)
            {
                o.setDelegate(del);
                return o;
            }
            return del;
        }
    }

    internal static class AttrsUtil
    {
        public static void
        addEndpointAttributes<T>(MetricsHelper<T>.AttributeResolver r, Type cl) where T : IceMX.Metrics
        {
            r.add("endpoint", cl.GetMethod("getEndpoint"));

            Type cli = typeof(Ice.EndpointInfo);
            r.add("endpointType", cl.GetMethod("getEndpointInfo"), cli.GetMethod("type"));
            r.add("endpointIsDatagram", cl.GetMethod("getEndpointInfo"), cli.GetMethod("datagram"));
            r.add("endpointIsSecure", cl.GetMethod("getEndpointInfo"), cli.GetMethod("secure"));
            r.add("endpointTimeout", cl.GetMethod("getEndpointInfo"), cli.GetField("timeout"));
            r.add("endpointCompress", cl.GetMethod("getEndpointInfo"), cli.GetField("compress"));

            cli = typeof(Ice.IPEndpointInfo);
            r.add("endpointHost", cl.GetMethod("getEndpointInfo"), cli.GetField("host"));
            r.add("endpointPort", cl.GetMethod("getEndpointInfo"), cli.GetField("port"));
        }

        public static void
        addConnectionAttributes<T>(MetricsHelper<T>.AttributeResolver r, Type cl) where T : IceMX.Metrics
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

            addEndpointAttributes<T>(r, cl);
        }
    }

    internal class ConnectionHelper : MetricsHelper<ConnectionMetrics>
    {
        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                try
                {
                    Type cl = typeof(ConnectionHelper);
                    add("parent", cl.GetMethod("getParent"));
                    add("id", cl.GetMethod("getId"));
                    add("state", cl.GetMethod("getState"));
                    AttrsUtil.addConnectionAttributes(this, cl);
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static AttributeResolver _attributes = new AttributeResolverI();

        public ConnectionHelper(Ice.ConnectionInfo con, Ice.Endpoint endpt, Ice.Instrumentation.ConnectionState state)
            : base(_attributes)
        {
            _connectionInfo = con;
            _endpoint = endpt;
            _state = state;
        }

        public string getId()
        {
            if (_id == null)
            {
                StringBuilder os = new StringBuilder();
                Ice.IPConnectionInfo info = getIPConnectionInfo();
                if (info != null)
                {
                    os.Append(info.localAddress).Append(':').Append(info.localPort);
                    os.Append(" -> ");
                    os.Append(info.remoteAddress).Append(':').Append(info.remotePort);
                }
                else
                {
                    os.Append("connection-").Append(_connectionInfo);
                }
                if (_connectionInfo.connectionId.Length > 0)
                {
                    os.Append(" [").Append(_connectionInfo.connectionId).Append("]");
                }
                _id = os.ToString();
            }
            return _id;
        }

        public string getState()
        {
            switch (_state)
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
            if (_connectionInfo.adapterName != null && _connectionInfo.adapterName.Length > 0)
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
            if (_endpointInfo == null)
            {
                _endpointInfo = _endpoint.getInfo();
            }
            return _endpointInfo;
        }

        private Ice.IPConnectionInfo
        getIPConnectionInfo()
        {
            for (Ice.ConnectionInfo p = _connectionInfo; p != null; p = p.underlying)
            {
                if (p is Ice.IPConnectionInfo)
                {
                    return (Ice.IPConnectionInfo)p;
                }
            }
            return null;
        }

        private readonly Ice.ConnectionInfo _connectionInfo;
        private readonly Ice.Endpoint _endpoint;
        private readonly Ice.Instrumentation.ConnectionState _state;
        private string _id;
        private Ice.EndpointInfo _endpointInfo;
    }

    internal class DispatchHelper : MetricsHelper<DispatchMetrics>
    {
        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                try
                {
                    Type cl = typeof(DispatchHelper);
                    add("parent", cl.GetMethod("getParent"));
                    add("id", cl.GetMethod("getId"));

                    AttrsUtil.addConnectionAttributes(this, cl);

                    Type clc = typeof(Ice.Current);
                    add("operation", cl.GetMethod("getCurrent"), clc.GetProperty("Operation"));
                    add("identity", cl.GetMethod("getIdentity"));
                    add("facet", cl.GetMethod("getCurrent"), clc.GetProperty("Facet"));
                    add("current", cl.GetMethod("getCurrent"), clc.GetProperty("RequestId"));
                    add("mode", cl.GetMethod("getMode"));
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static AttributeResolver _attributes = new AttributeResolverI();

        public DispatchHelper(Ice.Current current, int size) : base(_attributes)
        {
            _current = current;
            _size = size;
        }

        protected override string defaultResolve(string attribute)
        {
            if (attribute.IndexOf("context.", 0) == 0)
            {
                string v;
                if (_current.Context.TryGetValue(attribute.Substring(8), out v))
                {
                    return v;
                }
            }
            throw new ArgumentOutOfRangeException(attribute);
        }

        public override void initMetrics(DispatchMetrics v)
        {
            v.size += _size;
        }

        public string getMode()
        {
            return _current.RequestId == 0 ? "oneway" : "twoway";
        }

        public string getId()
        {
            if (_id == null)
            {
                StringBuilder os = new StringBuilder();
                if (_current.Id.category != null && _current.Id.category.Length > 0)
                {
                    os.Append(_current.Id.category).Append('/');
                }
                os.Append(_current.Id.name).Append(" [").Append(_current.Operation).Append(']');
                _id = os.ToString();
            }
            return _id;
        }

        public string getParent()
        {
            return _current.Adapter.GetName();
        }

        public Ice.ConnectionInfo getConnectionInfo()
        {
            if (_current.Connection != null)
            {
                return _current.Connection.getInfo();
            }
            return null;
        }

        public Ice.Endpoint getEndpoint()
        {
            if (_current.Connection != null)
            {
                return _current.Connection.getEndpoint();
            }
            return null;
        }

        public Ice.Connection getConnection()
        {
            return _current.Connection;
        }

        public Ice.EndpointInfo getEndpointInfo()
        {
            if (_current.Connection != null && _endpointInfo == null)
            {
                _endpointInfo = _current.Connection.getEndpoint().getInfo();
            }
            return _endpointInfo;
        }

        public Ice.Current getCurrent()
        {
            return _current;
        }

        public string getIdentity()
        {
            return _current.Id.ToString(_current.Adapter!.Communicator.ToStringMode);
        }

        private readonly Ice.Current _current;
        private readonly int _size;
        private string _id;
        private Ice.EndpointInfo _endpointInfo;
    }

    internal class InvocationHelper : MetricsHelper<InvocationMetrics>
    {
        internal class AttributeResolverI : AttributeResolver
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

                    Type cli = typeof(Ice.IObjectPrx);
                    add("facet", cl.GetMethod("getProxy"), cli.GetProperty("Facet"));
                    add("encoding", cl.GetMethod("getEncodingVersion"));
                    add("mode", cl.GetMethod("getMode"));
                    add("proxy", cl.GetMethod("getProxy"));
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static AttributeResolver _attributes = new AttributeResolverI();

        public InvocationHelper(Ice.IObjectPrx proxy, string op, Dictionary<string, string> ctx) : base(_attributes)
        {
            _proxy = proxy;
            _operation = op;
            _context = ctx;
        }

        protected override string defaultResolve(string attribute)
        {
            if (attribute.IndexOf("context.", 0) == 0)
            {
                string v;
                if (_context.TryGetValue(attribute.Substring(8), out v))
                {
                    return v;
                }
            }
            throw new ArgumentOutOfRangeException(attribute);
        }

        public string getMode()
        {
            if (_proxy == null)
            {
                throw new ArgumentOutOfRangeException("mode");
            }

            switch (_proxy.InvocationMode)
            {
                case InvocationMode.Twoway:
                    {
                        return "twoway";
                    }
                case InvocationMode.Oneway:
                    {
                        return "oneway";
                    }
                case InvocationMode.Datagram:
                    {
                        return "datagram";
                    }
                default:
                    {
                        // Note: it's not possible to invoke on a batch proxy, but it's
                        // possible to receive a batch request.
                        throw new ArgumentOutOfRangeException("mode");
                    }
            }
        }

        public string getId()
        {
            if (_id == null)
            {
                if (_proxy != null)
                {
                    StringBuilder os = new StringBuilder();
                    try
                    {
                        os.Append(_proxy.Clone(endpoints: emptyEndpoints)).Append(" [").Append(_operation).Append(']');
                    }
                    catch (Ice.Exception)
                    {
                        // Either a fixed proxy or the communicator is destroyed.
                        os.Append(_proxy.Identity.ToString(_proxy.Communicator.ToStringMode));
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

        public Ice.IObjectPrx getProxy()
        {
            return _proxy;
        }

        public string getEncodingVersion()
        {
            return Ice.Util.encodingVersionToString(_proxy.EncodingVersion);
        }

        public string getIdentity()
        {
            if (_proxy != null)
            {
                return _proxy.Identity.ToString(_proxy.Communicator.ToStringMode);
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

        private readonly Ice.IObjectPrx _proxy;
        private readonly string _operation;
        private readonly Dictionary<string, string> _context;
        private string _id;

        private static readonly Ice.Endpoint[] emptyEndpoints = Array.Empty<Ice.Endpoint>();
    }

    internal class ThreadHelper : MetricsHelper<ThreadMetrics>
    {
        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                try
                {
                    Type cl = typeof(ThreadHelper);
                    add("parent", cl.GetField("_parent"));
                    add("id", cl.GetField("_id"));
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static AttributeResolver _attributes = new AttributeResolverI();

        public ThreadHelper(string parent, string id, Ice.Instrumentation.ThreadState state) : base(_attributes)
        {
            _parent = parent;
            _id = id;
            _state = state;
        }

        public override void initMetrics(ThreadMetrics v)
        {
            switch (_state)
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

        public readonly string _parent;
        public readonly string _id;
        private readonly Ice.Instrumentation.ThreadState _state;
    }

    internal class EndpointHelper : MetricsHelper<Metrics>
    {
        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                try
                {
                    Type cl = typeof(EndpointHelper);
                    add("parent", cl.GetMethod("getParent"));
                    add("id", cl.GetMethod("getId"));
                    AttrsUtil.addEndpointAttributes(this, cl);
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static AttributeResolver _attributes = new AttributeResolverI();

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
            if (_endpointInfo == null)
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
            if (_id == null)
            {
                _id = _endpoint.ToString();
            }
            return _id;
        }

        public string getEndpoint()
        {
            return _endpoint.ToString();
        }

        private readonly Ice.Endpoint _endpoint;
        private string _id;
        private Ice.EndpointInfo _endpointInfo;
    }

    public class RemoteInvocationHelper : MetricsHelper<RemoteMetrics>
    {
        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                try
                {
                    Type cl = typeof(RemoteInvocationHelper);
                    add("parent", cl.GetMethod("getParent"));
                    add("id", cl.GetMethod("getId"));
                    add("requestId", cl.GetMethod("getRequestId"));
                    AttrsUtil.addConnectionAttributes(this, cl);
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static AttributeResolver _attributes = new AttributeResolverI();

        public RemoteInvocationHelper(Ice.ConnectionInfo con, Ice.Endpoint endpt, int requestId, int size) :
            base(_attributes)
        {
            _connectionInfo = con;
            _endpoint = endpt;
            _requestId = requestId;
            _size = size;
        }

        public override void initMetrics(RemoteMetrics v)
        {
            v.size += _size;
        }

        public string getId()
        {
            if (_id == null)
            {
                _id = _endpoint.ToString();
                if (_connectionInfo.connectionId != null && _connectionInfo.connectionId.Length > 0)
                {
                    _id += " [" + _connectionInfo.connectionId + "]";
                }
            }
            return _id;
        }

        public int getRequestId()
        {
            return _requestId;
        }

        public string getParent()
        {
            if (_connectionInfo.adapterName != null && _connectionInfo.adapterName.Length > 0)
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
            if (_endpointInfo == null)
            {
                _endpointInfo = _endpoint.getInfo();
            }
            return _endpointInfo;
        }

        private readonly Ice.ConnectionInfo _connectionInfo;
        private readonly Ice.Endpoint _endpoint;
        private readonly int _size;
        private readonly int _requestId;
        private string _id;
        private Ice.EndpointInfo _endpointInfo;
    }

    public class CollocatedInvocationHelper : MetricsHelper<CollocatedMetrics>
    {
        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                try
                {
                    Type cl = typeof(CollocatedInvocationHelper);
                    add("parent", cl.GetMethod("getParent"));
                    add("id", cl.GetMethod("getId"));
                    add("requestId", cl.GetMethod("getRequestId"));
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static AttributeResolver _attributes = new AttributeResolverI();

        public CollocatedInvocationHelper(Ice.ObjectAdapter adapter, int requestId, int size) :
            base(_attributes)
        {
            _id = adapter.GetName();
            _requestId = requestId;
            _size = size;
        }

        public override void initMetrics(CollocatedMetrics v)
        {
            v.size += _size;
        }

        public string getId()
        {
            return _id;
        }

        public int getRequestId()
        {
            return _requestId;
        }

        public string getParent()
        {
            return "Communicator";
        }

        private readonly int _size;
        private readonly int _requestId;
        private readonly string _id;
    }

    public class ObserverWithDelegateI : ObserverWithDelegate<Metrics, Ice.Instrumentation.Observer>
    {
    }

    public class ConnectionObserverI : ObserverWithDelegate<ConnectionMetrics, Ice.Instrumentation.ConnectionObserver>,
        Ice.Instrumentation.ConnectionObserver
    {
        public void sentBytes(int num)
        {
            _sentBytes = num;
            forEach(sentBytesUpdate);
            if (delegate_ != null)
            {
                delegate_.sentBytes(num);
            }
        }

        public void receivedBytes(int num)
        {
            _receivedBytes = num;
            forEach(receivedBytesUpdate);
            if (delegate_ != null)
            {
                delegate_.receivedBytes(num);
            }
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
    }

    public class DispatchObserverI : ObserverWithDelegate<DispatchMetrics, Ice.Instrumentation.DispatchObserver>,
        Ice.Instrumentation.DispatchObserver
    {
        public void
        userException()
        {
            forEach(userException);
            if (delegate_ != null)
            {
                delegate_.userException();
            }
        }

        public void reply(int size)
        {
            forEach((DispatchMetrics v) =>
            {
                v.replySize += size;
            });
            if (delegate_ != null)
            {
                delegate_.reply(size);
            }
        }

        private void userException(DispatchMetrics v)
        {
            ++v.userException;
        }
    }

    public class RemoteObserverI : ObserverWithDelegate<RemoteMetrics, Ice.Instrumentation.RemoteObserver>,
        Ice.Instrumentation.RemoteObserver
    {
        public void reply(int size)
        {
            forEach((RemoteMetrics v) =>
            {
                v.replySize += size;
            });
            if (delegate_ != null)
            {
                delegate_.reply(size);
            }
        }
    }

    public class CollocatedObserverI : ObserverWithDelegate<CollocatedMetrics, Ice.Instrumentation.CollocatedObserver>,
        Ice.Instrumentation.CollocatedObserver
    {
        public void reply(int size)
        {
            forEach((CollocatedMetrics v) =>
            {
                v.replySize += size;
            });
            if (delegate_ != null)
            {
                delegate_.reply(size);
            }
        }
    }

    public class InvocationObserverI : ObserverWithDelegate<InvocationMetrics, Ice.Instrumentation.InvocationObserver>,
        Ice.Instrumentation.InvocationObserver
    {
        public void
        userException()
        {
            forEach(userException);
            if (delegate_ != null)
            {
                delegate_.userException();
            }
        }

        public void
        retried()
        {
            forEach(incrementRetry);
            if (delegate_ != null)
            {
                delegate_.retried();
            }
        }

        public Ice.Instrumentation.RemoteObserver getRemoteObserver(Ice.ConnectionInfo con, Ice.Endpoint endpt,
                                                                    int requestId, int size)
        {
            Ice.Instrumentation.RemoteObserver del = null;
            if (delegate_ != null)
            {
                del = delegate_.getRemoteObserver(con, endpt, requestId, size);
            }
            return getObserver<RemoteMetrics, RemoteObserverI,
                Ice.Instrumentation.RemoteObserver>("Remote",
                                                    new RemoteInvocationHelper(con, endpt, requestId, size),
                                                    del);
        }

        public Ice.Instrumentation.CollocatedObserver getCollocatedObserver(Ice.ObjectAdapter adapter,
                                                                            int requestId,
                                                                            int size)
        {
            Ice.Instrumentation.CollocatedObserver del = null;
            if (delegate_ != null)
            {
                del = delegate_.getCollocatedObserver(adapter, requestId, size);
            }
            return getObserver<CollocatedMetrics, CollocatedObserverI,
                Ice.Instrumentation.CollocatedObserver>("Collocated",
                                                    new CollocatedInvocationHelper(adapter, requestId, size),
                                                    del);
        }

        private void incrementRetry(InvocationMetrics v)
        {
            ++v.retry;
        }

        private void userException(InvocationMetrics v)
        {
            ++v.userException;
        }
    }

    public class ThreadObserverI : ObserverWithDelegate<ThreadMetrics, Ice.Instrumentation.ThreadObserver>,
        Ice.Instrumentation.ThreadObserver
    {
        public void stateChanged(Ice.Instrumentation.ThreadState oldState, Ice.Instrumentation.ThreadState newState)
        {
            _oldState = oldState;
            _newState = newState;
            forEach(threadStateUpdate);
            if (delegate_ != null)
            {
                delegate_.stateChanged(oldState, newState);
            }
        }

        private void threadStateUpdate(ThreadMetrics v)
        {
            switch (_oldState)
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
            switch (_newState)
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
    }

    public class CommunicatorObserverI : Ice.Instrumentation.CommunicatorObserver
    {
        public CommunicatorObserverI(Communicator communicator, Logger logger)
        {
            _metrics = new MetricsAdminI(communicator, logger);
            _delegate = communicator.Observer;
            _connections = new ObserverFactoryWithDelegate<ConnectionMetrics, ConnectionObserverI,
                Ice.Instrumentation.ConnectionObserver>(_metrics, "Connection");
            _dispatch = new ObserverFactoryWithDelegate<DispatchMetrics, DispatchObserverI,
                Ice.Instrumentation.DispatchObserver>(_metrics, "Dispatch");
            _invocations = new ObserverFactoryWithDelegate<InvocationMetrics, InvocationObserverI,
                Ice.Instrumentation.InvocationObserver>(_metrics, "Invocation");
            _threads = new ObserverFactoryWithDelegate<ThreadMetrics, ThreadObserverI,
                Ice.Instrumentation.ThreadObserver>(_metrics, "Thread");
            _connects = new ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
                Ice.Instrumentation.Observer>(_metrics, "ConnectionEstablishment");
            _endpointLookups = new ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
                Ice.Instrumentation.Observer>(_metrics, "EndpointLookup");

            try
            {
                Type cl = typeof(InvocationMetrics);
                _invocations.registerSubMap<RemoteMetrics>("Remote", cl.GetField("remotes"));
                _invocations.registerSubMap<CollocatedMetrics>("Collocated", cl.GetField("collocated"));
            }
            catch (System.Exception)
            {
                Debug.Assert(false);
            }
        }

        public Ice.Instrumentation.Observer? getConnectionEstablishmentObserver(Endpoint endpt, string connector)
        {
            if (_connects.isEnabled())
            {
                try
                {
                    Ice.Instrumentation.Observer? del = null;
                    if (_delegate != null)
                    {
                        del = _delegate.getConnectionEstablishmentObserver(endpt, connector);
                    }
                    return _connects.getObserver(new EndpointHelper(endpt, connector), del);
                }
                catch (System.Exception ex)
                {
                    _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public Ice.Instrumentation.Observer? getEndpointLookupObserver(Endpoint endpt)
        {
            if (_endpointLookups.isEnabled())
            {
                try
                {
                    Ice.Instrumentation.Observer? del = null;
                    if (_delegate != null)
                    {
                        del = _delegate.getEndpointLookupObserver(endpt);
                    }
                    return _endpointLookups.getObserver(new EndpointHelper(endpt), del);
                }
                catch (System.Exception ex)
                {
                    _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public Ice.Instrumentation.ConnectionObserver? getConnectionObserver(ConnectionInfo c,
                                                                             Endpoint e,
                                                                             Ice.Instrumentation.ConnectionState s,
                                                                             Ice.Instrumentation.ConnectionObserver obsv)
        {
            if (_connections.isEnabled())
            {
                try
                {
                    Ice.Instrumentation.ConnectionObserver? del = null;
                    ConnectionObserverI? o = obsv is ConnectionObserverI ? (ConnectionObserverI)obsv : null;
                    if (_delegate != null)
                    {
                        del = _delegate.getConnectionObserver(c, e, s, o != null ? o.getDelegate() : obsv);
                    }
                    return _connections.getObserver(new ConnectionHelper(c, e, s), obsv, del);
                }
                catch (System.Exception ex)
                {
                    _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public Ice.Instrumentation.ThreadObserver getThreadObserver(string parent, string id,
                                                                    Ice.Instrumentation.ThreadState s,
                                                                    Ice.Instrumentation.ThreadObserver? obsv)
        {
            if (_threads.isEnabled())
            {
                try
                {
                    Ice.Instrumentation.ThreadObserver del = null;
                    ThreadObserverI? o = obsv is ThreadObserverI ? (ThreadObserverI)obsv : null;
                    if (_delegate != null)
                    {
                        del = _delegate.getThreadObserver(parent, id, s, o != null ? o.getDelegate() : obsv);
                    }
                    return _threads.getObserver(new ThreadHelper(parent, id, s), obsv, del);
                }
                catch (System.Exception ex)
                {
                    _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public Ice.Instrumentation.InvocationObserver getInvocationObserver(Ice.IObjectPrx prx, string operation,
                                                                            Dictionary<string, string> ctx)
        {
            if (_invocations.isEnabled())
            {
                try
                {
                    Ice.Instrumentation.InvocationObserver del = null;
                    if (_delegate != null)
                    {
                        del = _delegate.getInvocationObserver(prx, operation, ctx);
                    }
                    return _invocations.getObserver(new InvocationHelper(prx, operation, ctx), del);
                }
                catch (System.Exception ex)
                {
                    _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public Ice.Instrumentation.DispatchObserver getDispatchObserver(Ice.Current c, int size)
        {
            if (_dispatch.isEnabled())
            {
                try
                {
                    Ice.Instrumentation.DispatchObserver del = null;
                    if (_delegate != null)
                    {
                        del = _delegate.getDispatchObserver(c, size);
                    }
                    return _dispatch.getObserver(new DispatchHelper(c, size), del);
                }
                catch (System.Exception ex)
                {
                    _metrics.getLogger().error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public void setObserverUpdater(Ice.Instrumentation.ObserverUpdater updater)
        {
            if (updater == null)
            {
                _connections.setUpdater(null);
                _threads.setUpdater(null);
            }
            else
            {
                _connections.setUpdater(updater.updateConnectionObservers);
                _threads.setUpdater(updater.updateThreadObservers);
            }
            if (_delegate != null)
            {
                _delegate.setObserverUpdater(updater);
            }
        }

        public MetricsAdminI getFacet()
        {
            return _metrics;
        }

        private readonly MetricsAdminI _metrics;
        private readonly Ice.Instrumentation.CommunicatorObserver _delegate;
        private readonly ObserverFactoryWithDelegate<ConnectionMetrics, ConnectionObserverI,
            Ice.Instrumentation.ConnectionObserver> _connections;
        private readonly ObserverFactoryWithDelegate<DispatchMetrics, DispatchObserverI,
            Ice.Instrumentation.DispatchObserver> _dispatch;
        private readonly ObserverFactoryWithDelegate<InvocationMetrics, InvocationObserverI,
            Ice.Instrumentation.InvocationObserver> _invocations;
        private readonly ObserverFactoryWithDelegate<ThreadMetrics, ThreadObserverI,
            Ice.Instrumentation.ThreadObserver> _threads;
        private readonly ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
            Ice.Instrumentation.Observer> _connects;
        private readonly ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
            Ice.Instrumentation.Observer> _endpointLookups;
    }
}
