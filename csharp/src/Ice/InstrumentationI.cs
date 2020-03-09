//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using IceMX;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace IceInternal
{
    public class ObserverWithDelegate<T, O> : Observer<T>
        where T : Metrics, new()
        where O : class, Ice.Instrumentation.IObserver
    {
        public override void
        Attach()
        {
            base.Attach();
            if (Delegate != null)
            {
                Delegate.Attach();
            }
        }

        public override void
        Detach()
        {
            base.Detach();
            if (Delegate != null)
            {
                Delegate.Detach();
            }
        }

        public override void
        Failed(string exceptionName)
        {
            base.Failed(exceptionName);
            if (Delegate != null)
            {
                Delegate.Failed(exceptionName);
            }
        }

        public O?
        GetDelegate() => Delegate;

        public void
        SetDelegate(O? del) => Delegate = del;

        public Observer GetObserver<S, ObserverImpl, Observer>(string mapName, MetricsHelper<S> helper, Observer del)
            where S : Metrics, new()
            where ObserverImpl : ObserverWithDelegate<S, Observer>, Observer, new()
            where Observer : class, Ice.Instrumentation.IObserver
        {
            ObserverImpl? obsv = GetObserver<S, ObserverImpl>(mapName, helper);
            if (obsv != null)
            {
                obsv.SetDelegate(del);
                return obsv;
            }
            return del;
        }

        protected O? Delegate;
    }

    public class ObserverFactoryWithDelegate<T, OImpl, O> : ObserverFactory<T, OImpl>
        where T : Metrics, new()
        where OImpl : ObserverWithDelegate<T, O>, O, new()
        where O : class, Ice.Instrumentation.IObserver
    {
        public ObserverFactoryWithDelegate(MetricsAdminI metrics, string name) : base(metrics, name)
        {
        }

        public O? GetObserver(MetricsHelper<T> helper, O? del)
        {
            OImpl? o = GetObserver(helper);
            if (o != null)
            {
                o.SetDelegate(del);
                return o;
            }
            return del;
        }

        public O? GetObserver(MetricsHelper<T> helper, object? observer, O? del)
        {
            OImpl? o = GetObserver(helper, observer);
            if (o != null)
            {
                o.SetDelegate(del);
                return o;
            }
            return del;
        }
    }

    internal static class AttrsUtil
    {
        public static void
        AddEndpointAttributes<T>(MetricsHelper<T>.AttributeResolver r, Type cl) where T : IceMX.Metrics
        {
            r.Add("endpoint", cl.GetMethod("GetEndpoint"));

            Type cli = typeof(Ice.EndpointInfo);
            r.Add("endpointType", cl.GetMethod("GetEndpointInfo"), cli.GetMethod("Type"));
            r.Add("endpointIsDatagram", cl.GetMethod("GetEndpointInfo"), cli.GetMethod("Datagram"));
            r.Add("endpointIsSecure", cl.GetMethod("GetEndpointInfo"), cli.GetMethod("Secure"));
            r.Add("endpointTimeout", cl.GetMethod("GetEndpointInfo"), cli.GetField("Timeout"));
            r.Add("endpointCompress", cl.GetMethod("GetEndpointInfo"), cli.GetField("Compress"));

            cli = typeof(Ice.IPEndpointInfo);
            r.Add("endpointHost", cl.GetMethod("GetEndpointInfo"), cli.GetField("Host"));
            r.Add("endpointPort", cl.GetMethod("GetEndpointInfo"), cli.GetField("Port"));
        }

        public static void
        AddConnectionAttributes<T>(MetricsHelper<T>.AttributeResolver r, Type cl) where T : IceMX.Metrics
        {
            Type cli = typeof(Ice.ConnectionInfo);
            r.Add("incoming", cl.GetMethod("GetConnectionInfo"), cli.GetField("Incoming"));
            r.Add("adapterName", cl.GetMethod("GetConnectionInfo"), cli.GetField("AdapterName"));
            r.Add("connectionId", cl.GetMethod("GetConnectionInfo"), cli.GetField("ConnectionId"));

            cli = typeof(Ice.IPConnectionInfo);
            r.Add("localHost", cl.GetMethod("GetConnectionInfo"), cli.GetField("LocalAddress"));
            r.Add("localPort", cl.GetMethod("GetConnectionInfo"), cli.GetField("LocalPort"));
            r.Add("remoteHost", cl.GetMethod("GetConnectionInfo"), cli.GetField("RemoteAddress"));
            r.Add("remotePort", cl.GetMethod("GetConnectionInfo"), cli.GetField("RemotePort"));

            cli = typeof(Ice.UDPConnectionInfo);
            r.Add("mcastHost", cl.GetMethod("GetConnectionInfo"), cli.GetField("McastAddress"));
            r.Add("mcastPort", cl.GetMethod("GetConnectionInfo"), cli.GetField("McastPort"));

            AddEndpointAttributes<T>(r, cl);
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
                    Add("parent", cl.GetMethod("GetParent"));
                    Add("id", cl.GetMethod("GetId"));
                    Add("state", cl.GetMethod("GetState"));
                    AttrsUtil.AddConnectionAttributes(this, cl);
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public ConnectionHelper(ConnectionInfo con, IEndpoint endpt, Ice.Instrumentation.ConnectionState state)
            : base(_attributes)
        {
            _connectionInfo = con;
            _endpoint = endpt;
            _state = state;
        }

        public string GetId()
        {
            if (_id == null)
            {
                var os = new StringBuilder();
                IPConnectionInfo? info = GetIPConnectionInfo();
                if (info != null)
                {
                    os.Append(info.LocalAddress).Append(':').Append(info.LocalPort);
                    os.Append(" -> ");
                    os.Append(info.RemoteAddress).Append(':').Append(info.RemotePort);
                }
                else
                {
                    os.Append("connection-").Append(_connectionInfo);
                }
                if (_connectionInfo.ConnectionId.Length > 0)
                {
                    os.Append(" [").Append(_connectionInfo.ConnectionId).Append("]");
                }
                _id = os.ToString();
            }
            return _id;
        }

        public string GetState()
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

        public string GetParent()
        {
            if (_connectionInfo.AdapterName != null && _connectionInfo.AdapterName.Length > 0)
            {
                return _connectionInfo.AdapterName;
            }
            else
            {
                return "Communicator";
            }
        }

        public ConnectionInfo GetConnectionInfo() => _connectionInfo;

        public IEndpoint GetEndpoint() => _endpoint;

        public EndpointInfo GetEndpointInfo()
        {
            if (_endpointInfo == null)
            {
                _endpointInfo = _endpoint.GetInfo();
            }
            return _endpointInfo;
        }

        private IPConnectionInfo?
        GetIPConnectionInfo()
        {
            for (ConnectionInfo? p = _connectionInfo; p != null; p = p.Underlying)
            {
                if (p is IPConnectionInfo)
                {
                    return (IPConnectionInfo)p;
                }
            }
            return null;
        }

        private readonly ConnectionInfo _connectionInfo;
        private readonly IEndpoint _endpoint;
        private readonly Ice.Instrumentation.ConnectionState _state;
        private string? _id;
        private EndpointInfo? _endpointInfo;
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
                    Add("parent", cl.GetMethod("GetParent"));
                    Add("id", cl.GetMethod("GetId"));

                    AttrsUtil.AddConnectionAttributes(this, cl);

                    Type clc = typeof(Ice.Current);
                    Add("operation", cl.GetMethod("GetCurrent"), clc.GetProperty("Operation"));
                    Add("identity", cl.GetMethod("GetIdentity"));
                    Add("facet", cl.GetMethod("GetCurrent"), clc.GetProperty("Facet"));
                    Add("current", cl.GetMethod("GetCurrent"), clc.GetProperty("RequestId"));
                    Add("mode", cl.GetMethod("GetMode"));
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public DispatchHelper(Current current, int size) : base(_attributes)
        {
            _current = current;
            _size = size;
        }

        protected override string DefaultResolve(string attribute)
        {
            if (attribute.IndexOf("context.", 0) == 0)
            {
                if (_current.Context.TryGetValue(attribute.Substring(8), out string v))
                {
                    return v;
                }
            }
            throw new ArgumentOutOfRangeException(attribute);
        }

        public override void InitMetrics(DispatchMetrics v) => v.Size += _size;

        public string GetMode() => _current.RequestId == 0 ? "oneway" : "twoway";

        public string GetId()
        {
            if (_id == null)
            {
                var os = new StringBuilder();
                if (_current.Id.Category != null && _current.Id.Category.Length > 0)
                {
                    os.Append(_current.Id.Category).Append('/');
                }
                os.Append(_current.Id.Name).Append(" [").Append(_current.Operation).Append(']');
                _id = os.ToString();
            }
            return _id;
        }

        public string GetParent() => _current.Adapter.Name;

        public ConnectionInfo? GetConnectionInfo()
        {
            if (_current.Connection != null)
            {
                return _current.Connection.GetConnectionInfo();
            }
            return null;
        }

        public IEndpoint? GetEndpoint()
        {
            if (_current.Connection != null)
            {
                return _current.Connection.Endpoint;
            }
            return null;
        }

        public Connection? GetConnection() => _current.Connection;

        public EndpointInfo GetEndpointInfo()
        {
            if (_current.Connection != null && _endpointInfo == null)
            {
                _endpointInfo = _current.Connection.Endpoint.GetInfo();
            }
            return _endpointInfo;
        }

        public Current GetCurrent() => _current;

        public string GetIdentity() => _current.Id.ToString(_current.Adapter!.Communicator.ToStringMode);

        private readonly Current _current;
        private readonly int _size;
        private string? _id;
        private EndpointInfo? _endpointInfo;
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
                    Add("parent", cl.GetMethod("GetParent"));
                    Add("id", cl.GetMethod("GetId"));

                    Add("operation", cl.GetMethod("GetOperation"));
                    Add("identity", cl.GetMethod("GetIdentity"));

                    Type cli = typeof(Ice.IObjectPrx);
                    Add("facet", cl.GetMethod("GetProxy"), cli.GetProperty("Facet"));
                    Add("encoding", cl.GetMethod("GetEncoding"));
                    Add("mode", cl.GetMethod("GetMode"));
                    Add("proxy", cl.GetMethod("GetProxy"));
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public InvocationHelper(IObjectPrx? proxy, string op, Dictionary<string, string> ctx) : base(_attributes)
        {
            _proxy = proxy;
            _operation = op;
            _context = ctx;
        }

        protected override string DefaultResolve(string attribute)
        {
            if (attribute.IndexOf("context.", 0) == 0)
            {
                if (_context.TryGetValue(attribute.Substring(8), out string v))
                {
                    return v;
                }
            }
            throw new ArgumentOutOfRangeException(attribute);
        }

        public string GetMode()
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

        public string GetId()
        {
            if (_id == null)
            {
                if (_proxy != null)
                {
                    var os = new StringBuilder();
                    try
                    {
                        os.Append(_proxy.Clone(endpoints: _emptyEndpoints)).Append(" [").Append(_operation).Append(']');
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

        public string GetParent() => "Communicator";

        public IObjectPrx? GetProxy() => _proxy;

        public string GetEncoding() => _proxy.Encoding.ToString();

        public string GetIdentity()
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

        public string GetOperation() => _operation;

        private readonly IObjectPrx? _proxy;
        private readonly string _operation;
        private readonly Dictionary<string, string> _context;
        private string? _id;

        private static readonly IEndpoint[] _emptyEndpoints = Array.Empty<IEndpoint>();
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
                    Add("parent", cl.GetField("Parent"));
                    Add("id", cl.GetField("Id"));
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public ThreadHelper(string parent, string id, Ice.Instrumentation.ThreadState state) : base(_attributes)
        {
            Parent = parent;
            Id = id;
            _state = state;
        }

        public override void InitMetrics(ThreadMetrics v)
        {
            switch (_state)
            {
                case Ice.Instrumentation.ThreadState.ThreadStateInUseForIO:
                    ++v.InUseForIO;
                    break;
                case Ice.Instrumentation.ThreadState.ThreadStateInUseForUser:
                    ++v.InUseForUser;
                    break;
                case Ice.Instrumentation.ThreadState.ThreadStateInUseForOther:
                    ++v.InUseForOther;
                    break;
                default:
                    break;
            }
        }

        public readonly string Parent;
        public readonly string Id;
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
                    Add("parent", cl.GetMethod("GetParent"));
                    Add("id", cl.GetMethod("GetId"));
                    AttrsUtil.AddEndpointAttributes(this, cl);
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public EndpointHelper(IEndpoint endpt, string id) : base(_attributes)
        {
            _endpoint = endpt;
            _id = id;
        }

        public EndpointHelper(IEndpoint endpt) : base(_attributes) => _endpoint = endpt;

        public EndpointInfo GetEndpointInfo()
        {
            if (_endpointInfo == null)
            {
                _endpointInfo = _endpoint.GetInfo();
            }
            return _endpointInfo;
        }

        public string GetParent() => "Communicator";

        public string GetId()
        {
            if (_id == null)
            {
                _id = _endpoint.ToString();
            }
            return _id;
        }

        public string GetEndpoint() => _endpoint.ToString();

        private readonly IEndpoint _endpoint;
        private string? _id;
        private EndpointInfo? _endpointInfo;
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
                    Add("parent", cl.GetMethod("GetParent"));
                    Add("id", cl.GetMethod("GetId"));
                    Add("requestId", cl.GetMethod("GetRequestId"));
                    AttrsUtil.AddConnectionAttributes(this, cl);
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public RemoteInvocationHelper(Ice.ConnectionInfo con, Ice.IEndpoint endpt, int requestId, int size) :
            base(_attributes)
        {
            _connectionInfo = con;
            _endpoint = endpt;
            _requestId = requestId;
            _size = size;
        }

        public override void InitMetrics(RemoteMetrics v) => v.Size += _size;

        public string GetId()
        {
            if (_id == null)
            {
                _id = _endpoint.ToString();
                if (_connectionInfo.ConnectionId != null && _connectionInfo.ConnectionId.Length > 0)
                {
                    _id += " [" + _connectionInfo.ConnectionId + "]";
                }
            }
            return _id;
        }

        public int GetRequestId() => _requestId;

        public string GetParent()
        {
            if (_connectionInfo.AdapterName != null && _connectionInfo.AdapterName.Length > 0)
            {
                return _connectionInfo.AdapterName;
            }
            else
            {
                return "Communicator";
            }
        }

        public ConnectionInfo GetConnectionInfo() => _connectionInfo;

        public IEndpoint GetEndpoint() => _endpoint;

        public EndpointInfo GetEndpointInfo()
        {
            if (_endpointInfo == null)
            {
                _endpointInfo = _endpoint.GetInfo();
            }
            return _endpointInfo;
        }

        private readonly ConnectionInfo _connectionInfo;
        private readonly IEndpoint _endpoint;
        private readonly int _size;
        private readonly int _requestId;
        private string? _id;
        private EndpointInfo? _endpointInfo;
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
                    Add("parent", cl.GetMethod("GetParent"));
                    Add("id", cl.GetMethod("GetId"));
                    Add("requestId", cl.GetMethod("GetRequestId"));
                }
                catch (System.Exception)
                {
                    Debug.Assert(false);
                }
            }
        }
        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public CollocatedInvocationHelper(Ice.ObjectAdapter adapter, int requestId, int size) :
            base(_attributes)
        {
            _id = adapter.Name;
            _requestId = requestId;
            _size = size;
        }

        public override void InitMetrics(CollocatedMetrics v) => v.Size += _size;

        public string GetId() => _id;

        public int GetRequestId() => _requestId;

        public string GetParent() => "Communicator";

        private readonly int _size;
        private readonly int _requestId;
        private readonly string _id;
    }

    public class ObserverWithDelegateI : ObserverWithDelegate<Metrics, Ice.Instrumentation.IObserver>
    {
    }

    public class ConnectionObserverI : ObserverWithDelegate<ConnectionMetrics, Ice.Instrumentation.IConnectionObserver>,
        Ice.Instrumentation.IConnectionObserver
    {
        public void SentBytes(int num)
        {
            _sentBytes = num;
            ForEach(SentBytesUpdate);
            if (Delegate != null)
            {
                Delegate.SentBytes(num);
            }
        }

        public void ReceivedBytes(int num)
        {
            _receivedBytes = num;
            ForEach(ReceivedBytesUpdate);
            if (Delegate != null)
            {
                Delegate.ReceivedBytes(num);
            }
        }

        private void SentBytesUpdate(ConnectionMetrics v) => v.SentBytes += _sentBytes;

        private void ReceivedBytesUpdate(ConnectionMetrics v) => v.ReceivedBytes += _receivedBytes;

        private int _sentBytes;
        private int _receivedBytes;
    }

    public class DispatchObserverI : ObserverWithDelegate<DispatchMetrics, Ice.Instrumentation.IDispatchObserver>,
        Ice.Instrumentation.IDispatchObserver
    {
        public void
        RemoteException()
        {
            ForEach(RemoteException);
            if (Delegate != null)
            {
                Delegate.RemoteException();
            }
        }

        public void Reply(int size)
        {
            ForEach((DispatchMetrics v) => v.ReplySize += size);
            if (Delegate != null)
            {
                Delegate.Reply(size);
            }
        }

        private void RemoteException(DispatchMetrics v) => ++v.UserException;
    }

    public class RemoteObserverI : ObserverWithDelegate<RemoteMetrics, Ice.Instrumentation.IRemoteObserver>,
        Ice.Instrumentation.IRemoteObserver
    {
        public void Reply(int size)
        {
            ForEach((RemoteMetrics v) => v.ReplySize += size);
            if (Delegate != null)
            {
                Delegate.Reply(size);
            }
        }
    }

    public class CollocatedObserverI : ObserverWithDelegate<CollocatedMetrics, Ice.Instrumentation.ICollocatedObserver>,
        Ice.Instrumentation.ICollocatedObserver
    {
        public void Reply(int size)
        {
            ForEach((CollocatedMetrics v) => v.ReplySize += size);
            if (Delegate != null)
            {
                Delegate.Reply(size);
            }
        }
    }

    public class InvocationObserverI : ObserverWithDelegate<InvocationMetrics, Ice.Instrumentation.IInvocationObserver>,
        Ice.Instrumentation.IInvocationObserver
    {
        public void
        RemoteException()
        {
            ForEach(RemoteException);
            if (Delegate != null)
            {
                Delegate.RemoteException();
            }
        }

        public void
        Retried()
        {
            ForEach(IncrementRetry);
            if (Delegate != null)
            {
                Delegate.Retried();
            }
        }

        public Ice.Instrumentation.IRemoteObserver GetRemoteObserver(Ice.ConnectionInfo con, Ice.IEndpoint endpt,
                                                                    int requestId, int size)
        {
            Ice.Instrumentation.IRemoteObserver? del = null;
            if (Delegate != null)
            {
                del = Delegate.GetRemoteObserver(con, endpt, requestId, size);
            }
            return GetObserver<RemoteMetrics, RemoteObserverI,
                Ice.Instrumentation.IRemoteObserver>("Remote",
                                                    new RemoteInvocationHelper(con, endpt, requestId, size),
                                                    del);
        }

        public Ice.Instrumentation.ICollocatedObserver GetCollocatedObserver(Ice.ObjectAdapter adapter,
                                                                            int requestId,
                                                                            int size)
        {
            Ice.Instrumentation.ICollocatedObserver? del = null;
            if (Delegate != null)
            {
                del = Delegate.GetCollocatedObserver(adapter, requestId, size);
            }
            return GetObserver<CollocatedMetrics, CollocatedObserverI,
                Ice.Instrumentation.ICollocatedObserver>("Collocated",
                                                    new CollocatedInvocationHelper(adapter, requestId, size),
                                                    del);
        }

        private void IncrementRetry(InvocationMetrics v) => ++v.Retry;

        private void RemoteException(InvocationMetrics v) => ++v.UserException;
    }

    public class ThreadObserverI : ObserverWithDelegate<ThreadMetrics, Ice.Instrumentation.IThreadObserver>,
        Ice.Instrumentation.IThreadObserver
    {
        public void StateChanged(Ice.Instrumentation.ThreadState oldState, Ice.Instrumentation.ThreadState newState)
        {
            _oldState = oldState;
            _newState = newState;
            ForEach(ThreadStateUpdate);
            if (Delegate != null)
            {
                Delegate.StateChanged(oldState, newState);
            }
        }

        private void ThreadStateUpdate(ThreadMetrics v)
        {
            switch (_oldState)
            {
                case Ice.Instrumentation.ThreadState.ThreadStateInUseForIO:
                    --v.InUseForIO;
                    break;
                case Ice.Instrumentation.ThreadState.ThreadStateInUseForUser:
                    --v.InUseForUser;
                    break;
                case Ice.Instrumentation.ThreadState.ThreadStateInUseForOther:
                    --v.InUseForOther;
                    break;
                default:
                    break;
            }
            switch (_newState)
            {
                case Ice.Instrumentation.ThreadState.ThreadStateInUseForIO:
                    ++v.InUseForIO;
                    break;
                case Ice.Instrumentation.ThreadState.ThreadStateInUseForUser:
                    ++v.InUseForUser;
                    break;
                case Ice.Instrumentation.ThreadState.ThreadStateInUseForOther:
                    ++v.InUseForOther;
                    break;
                default:
                    break;
            }
        }

        private Ice.Instrumentation.ThreadState _oldState;
        private Ice.Instrumentation.ThreadState _newState;
    }

    public class CommunicatorObserverI : Ice.Instrumentation.ICommunicatorObserver
    {
        public CommunicatorObserverI(Communicator communicator, ILogger logger)
        {
            _metrics = new MetricsAdminI(communicator, logger);
            _delegate = communicator.Observer;
            _connections = new ObserverFactoryWithDelegate<ConnectionMetrics, ConnectionObserverI,
                Ice.Instrumentation.IConnectionObserver>(_metrics, "Connection");
            _dispatch = new ObserverFactoryWithDelegate<DispatchMetrics, DispatchObserverI,
                Ice.Instrumentation.IDispatchObserver>(_metrics, "Dispatch");
            _invocations = new ObserverFactoryWithDelegate<InvocationMetrics, InvocationObserverI,
                Ice.Instrumentation.IInvocationObserver>(_metrics, "Invocation");
            _threads = new ObserverFactoryWithDelegate<ThreadMetrics, ThreadObserverI,
                Ice.Instrumentation.IThreadObserver>(_metrics, "Thread");
            _connects = new ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
                Ice.Instrumentation.IObserver>(_metrics, "ConnectionEstablishment");
            _endpointLookups = new ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
                Ice.Instrumentation.IObserver>(_metrics, "EndpointLookup");

            try
            {
                Type cl = typeof(InvocationMetrics);
                _invocations.RegisterSubMap<RemoteMetrics>("Remote", cl.GetField("Remotes"));
                _invocations.RegisterSubMap<CollocatedMetrics>("Collocated", cl.GetField("Collocated"));
            }
            catch (System.Exception)
            {
                Debug.Assert(false);
            }
        }

        public Ice.Instrumentation.IObserver? GetConnectionEstablishmentObserver(IEndpoint endpt, string connector)
        {
            if (_connects.IsEnabled())
            {
                try
                {
                    Ice.Instrumentation.IObserver? del = null;
                    if (_delegate != null)
                    {
                        del = _delegate.GetConnectionEstablishmentObserver(endpt, connector);
                    }
                    return _connects.GetObserver(new EndpointHelper(endpt, connector), del);
                }
                catch (System.Exception ex)
                {
                    _metrics.GetLogger().Error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public Ice.Instrumentation.IObserver? GetEndpointLookupObserver(IEndpoint endpt)
        {
            if (_endpointLookups.IsEnabled())
            {
                try
                {
                    Ice.Instrumentation.IObserver? del = null;
                    if (_delegate != null)
                    {
                        del = _delegate.GetEndpointLookupObserver(endpt);
                    }
                    return _endpointLookups.GetObserver(new EndpointHelper(endpt), del);
                }
                catch (System.Exception ex)
                {
                    _metrics.GetLogger().Error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public Ice.Instrumentation.IConnectionObserver? GetConnectionObserver(ConnectionInfo c,
                                                                             IEndpoint e,
                                                                             Ice.Instrumentation.ConnectionState s,
                                                                             Ice.Instrumentation.IConnectionObserver? obsv)
        {
            if (_connections.IsEnabled())
            {
                try
                {
                    Ice.Instrumentation.IConnectionObserver? del = null;
                    ConnectionObserverI? o = obsv is ConnectionObserverI ? (ConnectionObserverI)obsv : null;
                    if (_delegate != null)
                    {
                        del = _delegate.GetConnectionObserver(c, e, s, o != null ? o.GetDelegate() : obsv);
                    }
                    return _connections.GetObserver(new ConnectionHelper(c, e, s), obsv, del);
                }
                catch (System.Exception ex)
                {
                    _metrics.GetLogger().Error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public Ice.Instrumentation.IThreadObserver? GetThreadObserver(string parent, string id,
                                                                      Ice.Instrumentation.ThreadState s,
                                                                      Ice.Instrumentation.IThreadObserver? oldObsv)
        {
            if (_threads.IsEnabled())
            {
                try
                {
                    Ice.Instrumentation.IThreadObserver? newDelegate = null;
                    if (_delegate != null)
                    {
                        ThreadObserverI? o = oldObsv is ThreadObserverI ? (ThreadObserverI)oldObsv : null;
                        newDelegate = _delegate.GetThreadObserver(parent, id, s, o != null ? o.GetDelegate() : oldObsv);
                    }
                    return _threads.GetObserver(new ThreadHelper(parent, id, s), oldObsv, newDelegate);
                }
                catch (System.Exception ex)
                {
                    _metrics.GetLogger().Error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public Ice.Instrumentation.IInvocationObserver? GetInvocationObserver(IObjectPrx? prx, string operation,
                                                                              Dictionary<string, string> ctx)
        {
            if (_invocations.IsEnabled())
            {
                try
                {
                    Ice.Instrumentation.IInvocationObserver? del = null;
                    if (_delegate != null)
                    {
                        del = _delegate.GetInvocationObserver(prx, operation, ctx);
                    }
                    return _invocations.GetObserver(new InvocationHelper(prx, operation, ctx), del);
                }
                catch (System.Exception ex)
                {
                    _metrics.GetLogger().Error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public Ice.Instrumentation.IDispatchObserver? GetDispatchObserver(Current c, int size)
        {
            if (_dispatch.IsEnabled())
            {
                try
                {
                    Ice.Instrumentation.IDispatchObserver? del = null;
                    if (_delegate != null)
                    {
                        del = _delegate.GetDispatchObserver(c, size);
                    }
                    return _dispatch.GetObserver(new DispatchHelper(c, size), del);
                }
                catch (System.Exception ex)
                {
                    _metrics.GetLogger().Error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public void SetObserverUpdater(Ice.Instrumentation.IObserverUpdater? updater)
        {
            if (updater == null)
            {
                _connections.SetUpdater(null);
                _threads.SetUpdater(null);
            }
            else
            {
                _connections.SetUpdater(updater.UpdateConnectionObservers);
                _threads.SetUpdater(updater.UpdateThreadObservers);
            }
            if (_delegate != null)
            {
                _delegate.SetObserverUpdater(updater);
            }
        }

        public MetricsAdminI GetFacet() => _metrics;

        private readonly MetricsAdminI _metrics;
        private readonly Ice.Instrumentation.ICommunicatorObserver? _delegate;
        private readonly ObserverFactoryWithDelegate<ConnectionMetrics, ConnectionObserverI,
            Ice.Instrumentation.IConnectionObserver> _connections;
        private readonly ObserverFactoryWithDelegate<DispatchMetrics, DispatchObserverI,
            Ice.Instrumentation.IDispatchObserver> _dispatch;
        private readonly ObserverFactoryWithDelegate<InvocationMetrics, InvocationObserverI,
            Ice.Instrumentation.IInvocationObserver> _invocations;
        private readonly ObserverFactoryWithDelegate<ThreadMetrics, ThreadObserverI,
            Ice.Instrumentation.IThreadObserver> _threads;
        private readonly ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
            Ice.Instrumentation.IObserver> _connects;
        private readonly ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
            Ice.Instrumentation.IObserver> _endpointLookups;
    }
}
