//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Text;
using ZeroC.Ice.Instrumentation;
using ZeroC.IceMX;

namespace ZeroC.IceMX
{
    // Extends generated class that requires a public parameterless constructor in the code below.
    public partial class InvocationMetrics
    {
        public InvocationMetrics()
            : this(remotes: Array.Empty<Metrics>(), collocated: Array.Empty<Metrics>())
        {
        }
    }
}

namespace ZeroC.Ice
{
    internal class ObserverWithDelegate<T, O> : Observer<T>
        where T : Metrics, new()
        where O : class, IObserver
    {
        public O? Delegate { get; set; }

        public override void Attach()
        {
            base.Attach();
            Delegate?.Attach();
        }

        public override void Detach()
        {
            base.Detach();
            Delegate?.Detach();
        }

        public override void Failed(string exceptionName)
        {
            base.Failed(exceptionName);
            Delegate?.Failed(exceptionName);
        }

        public Observer? GetObserver<S, ObserverImpl, Observer>(string mapName, MetricsHelper<S> helper, Observer? del)
            where S : Metrics, new()
            where ObserverImpl : ObserverWithDelegate<S, Observer>, Observer, new()
            where Observer : class, IObserver
        {
            ObserverImpl? obsv = GetObserver<S, ObserverImpl>(mapName, helper);
            if (obsv != null)
            {
                obsv.Delegate = del;
                return obsv;
            }
            return del;
        }
    }

    internal class ObserverFactoryWithDelegate<T, OImpl, O> : ObserverFactory<T, OImpl>
        where T : Metrics, new()
        where OImpl : ObserverWithDelegate<T, O>, O, new()
        where O : class, IObserver
    {
        public ObserverFactoryWithDelegate(MetricsAdminI metrics, string name)
            : base(metrics, name)
        {
        }

        public O? GetObserver(MetricsHelper<T> helper, O? del)
        {
            OImpl? o = GetObserver(helper);
            if (o != null)
            {
                o.Delegate = del;
                return o;
            }
            return del;
        }

        public O? GetObserver(MetricsHelper<T> helper, object? observer, O? del)
        {
            OImpl? o = GetObserver(helper, observer);
            if (o != null)
            {
                o.Delegate = del;
                return o;
            }
            return del;
        }
    }

    internal class ConnectionHelper : MetricsHelper<ConnectionMetrics>
    {
        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                Add("parent", obj => (obj as ConnectionHelper)?.Parent);
                Add("id", obj => (obj as ConnectionHelper)?.Id);
                Add("state", obj => (obj as ConnectionHelper)?.State.ToString().ToLowerInvariant());
                Add("incoming", obj => (obj as ConnectionHelper)?.ConnectionInfo.Incoming);
                Add("adapterName", obj => (obj as ConnectionHelper)?.ConnectionInfo.AdapterName);
                Add("connectionId", obj => (obj as ConnectionHelper)?.ConnectionInfo.ConnectionId);

                Add("localHost", obj => (obj as ConnectionHelper)?.IPConnectionInfo?.LocalAddress);
                Add("localPort", obj => (obj as ConnectionHelper)?.IPConnectionInfo?.LocalPort);
                Add("remoteHost", obj => (obj as ConnectionHelper)?.IPConnectionInfo?.RemoteAddress);
                Add("remotePort", obj => (obj as ConnectionHelper)?.IPConnectionInfo?.RemotePort);

                Add("mcastHost",
                    obj => ((obj as ConnectionHelper)?.ConnectionInfo as UDPConnectionInfo)?.McastAddress);
                Add("mcastPort",
                    obj => ((obj as ConnectionHelper)?.ConnectionInfo as UDPConnectionInfo)?.McastPort);

                Add("endpoint", obj => (obj as ConnectionHelper)?.Endpoint);
                Add("endpointType", obj => (obj as ConnectionHelper)?.Endpoint?.Type);
                Add("endpointIsDatagram", obj => (obj as ConnectionHelper)?.Endpoint?.IsDatagram);
                Add("endpointIsSecure", obj => (obj as ConnectionHelper)?.Endpoint?.IsSecure);
                Add("endpointTimeout", obj => (obj as ConnectionHelper)?.Endpoint?.Timeout);
                Add("endpointCompress", obj => (obj as ConnectionHelper)?.Endpoint?.HasCompressionFlag);
                Add("endpointHost", obj => ((obj as ConnectionHelper)?.Endpoint as IPEndpoint)?.Host);
                Add("endpointPort", obj => ((obj as ConnectionHelper)?.Endpoint as IPEndpoint)?.Port);
            }
        }
        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public ConnectionHelper(ConnectionInfo con, Endpoint endpt, ConnectionState state)
            : base(_attributes)
        {
            ConnectionInfo = con;
            Endpoint = endpt;
            State = state;
        }

        public string Id
        {
            get
            {
                if (_id == null)
                {
                    var os = new StringBuilder();
                    IPConnectionInfo? info = IPConnectionInfo;
                    if (info != null)
                    {
                        os.Append(info.LocalAddress).Append(':').Append(info.LocalPort);
                        os.Append(" -> ");
                        os.Append(info.RemoteAddress).Append(':').Append(info.RemotePort);
                    }
                    else
                    {
                        os.Append("connection-").Append(ConnectionInfo);
                    }
                    if (ConnectionInfo.ConnectionId.Length > 0)
                    {
                        os.Append(" [").Append(ConnectionInfo.ConnectionId).Append("]");
                    }
                    _id = os.ToString();
                }
                return _id;
            }
        }

        public ConnectionState State { get; }

        public string Parent =>
            string.IsNullOrEmpty(ConnectionInfo.AdapterName) ? "Communicator" : ConnectionInfo.AdapterName;

        public ConnectionInfo ConnectionInfo { get; }

        public Endpoint Endpoint { get; }

        // TODO temporary until Underlying gets removed
        internal IPConnectionInfo? IPConnectionInfo
        {
            get
            {
                for (ConnectionInfo? p = ConnectionInfo; p != null; p = p.Underlying)
                {
                    if (p is IPConnectionInfo info)
                    {
                        return info;
                    }
                }
                return null;
            }
        }

        private string? _id;
    }

    internal class DispatchHelper : MetricsHelper<DispatchMetrics>
    {
        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                Add("parent", obj => (obj as DispatchHelper)?.Parent);
                Add("id", obj => (obj as DispatchHelper)?.Id);

                Add("incoming", obj => (obj as DispatchHelper)?.ConnectionInfo?.Incoming);
                Add("adapterName", obj => (obj as DispatchHelper)?.ConnectionInfo?.AdapterName);
                Add("connectionId", obj => (obj as DispatchHelper)?.ConnectionInfo?.ConnectionId);

                Add("localHost", obj => (obj as DispatchHelper)?.IPConnectionInfo?.LocalAddress);
                Add("localPort", obj => (obj as DispatchHelper)?.IPConnectionInfo?.LocalPort);
                Add("remoteHost", obj => (obj as DispatchHelper)?.IPConnectionInfo?.RemoteAddress);
                Add("remotePort", obj => (obj as DispatchHelper)?.IPConnectionInfo?.RemotePort);

                Add("mcastHost",
                    obj => ((obj as DispatchHelper)?.ConnectionInfo as UDPConnectionInfo)?.McastAddress);
                Add("mcastPort",
                    obj => ((obj as DispatchHelper)?.ConnectionInfo as UDPConnectionInfo)?.McastPort);

                Add("endpoint", obj => (obj as DispatchHelper)?.Endpoint);
                Add("endpointType", obj => (obj as DispatchHelper)?.Endpoint?.Type);
                Add("endpointIsDatagram", obj => (obj as DispatchHelper)?.Endpoint?.IsDatagram);
                Add("endpointIsSecure", obj => (obj as DispatchHelper)?.Endpoint?.IsSecure);
                Add("endpointTimeout", obj => (obj as DispatchHelper)?.Endpoint?.Timeout);
                Add("endpointCompress", obj => (obj as DispatchHelper)?.Endpoint?.HasCompressionFlag);
                Add("endpointHost", obj => ((obj as DispatchHelper)?.Endpoint as IPEndpoint)?.Host);
                Add("endpointPort", obj => ((obj as DispatchHelper)?.Endpoint as IPEndpoint)?.Port);

                Add("operation", obj => (obj as DispatchHelper)?.Current.Operation);
                Add("identity", obj => (obj as DispatchHelper)?.Identity);
                Add("facet", obj => (obj as DispatchHelper)?.Current.Facet);
                Add("requestId", obj => (obj as DispatchHelper)?.Current.RequestId);
                Add("mode", obj => (obj as DispatchHelper)?.Mode);
            }
        }
        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public DispatchHelper(Current current, int size)
            : base(_attributes)
        {
            Current = current;
            _size = size;
        }

        protected override string DefaultResolve(string attribute)
        {
            if (attribute.StartsWith("context.") && Current.Context.TryGetValue(attribute.Substring(8), out string? v))
            {
                return v;
            }
            throw new ArgumentOutOfRangeException(attribute);
        }

        public override void InitMetrics(DispatchMetrics v) => v.Size += _size;

        public string Mode => Current.RequestId == 0 ? "oneway" : "twoway";

        public string Id
        {
            get
            {
                _id ??= $"{Current.Identity} [{Current.Operation}]";
                return _id;
            }
        }

        public string Parent => Current.Adapter.Name;

        public ConnectionInfo? ConnectionInfo => Current.Connection?.GetConnectionInfo();

        // TODO temporary until Underlying gets removed
        internal IPConnectionInfo? IPConnectionInfo
        {
            get
            {
                for (ConnectionInfo? p = ConnectionInfo; p != null; p = p.Underlying)
                {
                    if (p is IPConnectionInfo info)
                    {
                        return info;
                    }
                }
                return null;
            }
        }

        public Endpoint? Endpoint => Current.Connection?.Endpoint ?? throw new ArgumentOutOfRangeException();

        public Current Current { get; }

        public string Identity => Current.Identity.ToString(Current.Adapter!.Communicator.ToStringMode);

        private readonly int _size;
        private string? _id;
    }

    internal class InvocationHelper : MetricsHelper<InvocationMetrics>
    {
        internal class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                Add("parent", obj => (obj as InvocationHelper)?.Parent);
                Add("id", obj => (obj as InvocationHelper)?.Id);

                Add("operation", obj => (obj as InvocationHelper)?.Operation);
                Add("identity", obj => (obj as InvocationHelper)?.Identity);

                Add("facet", obj => (obj as InvocationHelper)?.Proxy.Facet);
                Add("encoding", obj => (obj as InvocationHelper)?.Proxy.Encoding);
                Add("mode", obj => (obj as InvocationHelper)?.Mode);
                Add("proxy", obj => (obj as InvocationHelper)?.Proxy);
            }
        }
        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public InvocationHelper(IObjectPrx proxy, string op, IReadOnlyDictionary<string, string> ctx)
            : base(_attributes)
        {
            Proxy = proxy;
            Operation = op;
            _context = ctx;
        }

        protected override string DefaultResolve(string attribute)
        {
            if (attribute.StartsWith("context.") && _context.TryGetValue(attribute.Substring(8), out string? v))
            {
                return v;
            }
            throw new ArgumentOutOfRangeException(attribute);
        }

        public string Mode => Proxy.InvocationMode.ToString().ToLowerInvariant();

        public string Id
        {
            get
            {
                if (_id == null)
                {
                    var sb = new StringBuilder();
                    try
                    {
                        sb.Append(Proxy.Clone(endpoints: Array.Empty<Endpoint>()));
                        sb.Append(" [").Append(Operation).Append(']');
                    }
                    catch (Exception)
                    {
                        // Either a fixed proxy or the communicator is destroyed.
                        sb.Append(Proxy.Identity.ToString(Proxy.Communicator.ToStringMode));
                        sb.Append(" [").Append(Operation).Append(']');
                    }
                    _id = sb.ToString();
                }
                return _id;
            }
        }

        public string Parent => "Communicator";

        public IObjectPrx Proxy { get; }

        public string Identity => Proxy?.Identity.ToString(Proxy.Communicator.ToStringMode) ?? "";

        public string Operation { get; }

        private readonly IReadOnlyDictionary<string, string> _context;
        private string? _id;
    }

    internal class EndpointHelper : MetricsHelper<Metrics>
    {
        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                Add("parent", obj => "Communicator");
                Add("id", obj => (obj as EndpointHelper)?.Id);
                Add("endpoint", obj => (obj as EndpointHelper)?.Endpoint);
                Add("endpointType", obj => (obj as EndpointHelper)?.Endpoint?.Type);
                Add("endpointIsDatagram", obj => (obj as EndpointHelper)?.Endpoint?.IsDatagram);
                Add("endpointIsSecure", obj => (obj as EndpointHelper)?.Endpoint?.IsSecure);
                Add("endpointTimeout", obj => (obj as EndpointHelper)?.Endpoint?.Timeout);
                Add("endpointCompress", obj => (obj as EndpointHelper)?.Endpoint?.HasCompressionFlag);
                Add("endpointHost", obj => ((obj as EndpointHelper)?.Endpoint as IPEndpoint)?.Host);
                Add("endpointPort", obj => ((obj as EndpointHelper)?.Endpoint as IPEndpoint)?.Port);
            }
        }

        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public EndpointHelper(Endpoint endpt, string id)
            : base(_attributes)
        {
            Endpoint = endpt;
            _id = id;
        }

        public EndpointHelper(Endpoint endpt)
            : base(_attributes) => Endpoint = endpt;

        public string Id
        {
            get
            {
                _id ??= Endpoint.ToString();
                return _id;
            }
        }

        public Endpoint Endpoint { get; }

        private string? _id;
    }

    public class RemoteInvocationHelper : MetricsHelper<RemoteMetrics>
    {
        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                Add("parent", obj => (obj as RemoteInvocationHelper)?.Parent);
                Add("id", obj => (obj as RemoteInvocationHelper)?.Id);
                Add("requestId", obj => (obj as RemoteInvocationHelper)?.RequestId);
                Add("incoming", obj => (obj as RemoteInvocationHelper)?.ConnectionInfo.Incoming);
                Add("adapterName", obj => (obj as RemoteInvocationHelper)?.ConnectionInfo.AdapterName);
                Add("connectionId", obj => (obj as RemoteInvocationHelper)?.ConnectionInfo.ConnectionId);

                Add("localHost", obj => (obj as RemoteInvocationHelper)?.IPConnectionInfo?.LocalAddress);
                Add("localPort", obj => (obj as RemoteInvocationHelper)?.IPConnectionInfo?.LocalPort);
                Add("remoteHost", obj => (obj as RemoteInvocationHelper)?.IPConnectionInfo?.RemoteAddress);
                Add("remotePort", obj => (obj as RemoteInvocationHelper)?.IPConnectionInfo?.RemotePort);

                Add("mcastHost",
                    obj => ((obj as RemoteInvocationHelper)?.ConnectionInfo as UDPConnectionInfo)?.McastAddress);
                Add("mcastPort",
                    obj => ((obj as RemoteInvocationHelper)?.ConnectionInfo as UDPConnectionInfo)?.McastPort);
                Add("endpoint", obj => (obj as RemoteInvocationHelper)?.Endpoint);
                Add("endpointType", obj => (obj as RemoteInvocationHelper)?.Endpoint?.Type);
                Add("endpointIsDatagram", obj => (obj as RemoteInvocationHelper)?.Endpoint?.IsDatagram);
                Add("endpointIsSecure", obj => (obj as RemoteInvocationHelper)?.Endpoint?.IsSecure);
                Add("endpointTimeout", obj => (obj as RemoteInvocationHelper)?.Endpoint?.Timeout);
                Add("endpointCompress", obj => (obj as RemoteInvocationHelper)?.Endpoint?.HasCompressionFlag);
                Add("endpointHost", obj => ((obj as RemoteInvocationHelper)?.Endpoint as IPEndpoint)?.Host);
                Add("endpointPort", obj => ((obj as RemoteInvocationHelper)?.Endpoint as IPEndpoint)?.Port);
            }
        }
        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public RemoteInvocationHelper(ConnectionInfo con, Endpoint endpt, int requestId, int size)
            : base(_attributes)
        {
            ConnectionInfo = con;
            Endpoint = endpt;
            RequestId = requestId;
            _size = size;
        }

        public override void InitMetrics(RemoteMetrics v) => v.Size += _size;

        public string Id
        {
            get
            {
                if (_id == null)
                {
                    _id = string.IsNullOrEmpty(ConnectionInfo.ConnectionId) ?
                        Endpoint.ToString() : $"{Endpoint} [" + ConnectionInfo.ConnectionId + "]";
                }
                return _id;
            }
        }

        public int RequestId { get; }

        public string Parent =>
            string.IsNullOrEmpty(ConnectionInfo.AdapterName) ? "Communicator" : ConnectionInfo.AdapterName;

        public ConnectionInfo ConnectionInfo { get; }

        // TODO temporary until Underlying gets removed
        internal IPConnectionInfo? IPConnectionInfo
        {
            get
            {
                for (ConnectionInfo? p = ConnectionInfo; p != null; p = p.Underlying)
                {
                    if (p is IPConnectionInfo info)
                    {
                        return info;
                    }
                }
                return null;
            }
        }

        public Endpoint Endpoint { get; }

        private readonly int _size;
        private string? _id;
    }

    public class CollocatedInvocationHelper : MetricsHelper<CollocatedMetrics>
    {
        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                Add("parent", obj => "Communicator");
                Add("id", obj => (obj as CollocatedInvocationHelper)?.Id);
                Add("requestId", obj => (obj as CollocatedInvocationHelper)?.RequestId);
            }
        }
        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        public CollocatedInvocationHelper(ObjectAdapter adapter, int requestId, int size)
            : base(_attributes)
        {
            Id = adapter.Name;
            RequestId = requestId;
            _size = size;
        }

        public override void InitMetrics(CollocatedMetrics v) => v.Size += _size;

        public string Id { get; }

        public int RequestId { get; }

        private readonly int _size;
    }

    internal class ObserverWithDelegateI : ObserverWithDelegate<Metrics, IObserver>
    {
    }

    internal class ConnectionObserverI : ObserverWithDelegate<ConnectionMetrics, IConnectionObserver>,
        IConnectionObserver
    {
        public void SentBytes(int num)
        {
            _sentBytes = num;
            ForEach(SentBytesUpdate);
            Delegate?.SentBytes(num);
        }

        public void ReceivedBytes(int num)
        {
            _receivedBytes = num;
            ForEach(ReceivedBytesUpdate);
            Delegate?.ReceivedBytes(num);
        }

        private void SentBytesUpdate(ConnectionMetrics v) => v.SentBytes += _sentBytes;

        private void ReceivedBytesUpdate(ConnectionMetrics v) => v.ReceivedBytes += _receivedBytes;

        private int _sentBytes;
        private int _receivedBytes;
    }

    internal class DispatchObserverI : ObserverWithDelegate<DispatchMetrics, IDispatchObserver>, IDispatchObserver
    {
        public void RemoteException()
        {
            ForEach(RemoteException);
            Delegate?.RemoteException();
        }

        public void Reply(int size)
        {
            ForEach((DispatchMetrics v) => v.ReplySize += size);
            Delegate?.Reply(size);
        }

        private void RemoteException(DispatchMetrics v) => ++v.UserException;
    }

    internal class RemoteObserverI : ObserverWithDelegate<RemoteMetrics, IRemoteObserver>, IRemoteObserver
    {
        public void Reply(int size)
        {
            ForEach(v => v.ReplySize += size);
            Delegate?.Reply(size);
        }
    }

    internal class CollocatedObserverI : ObserverWithDelegate<CollocatedMetrics, ICollocatedObserver>,
        ICollocatedObserver
    {
        public void Reply(int size)
        {
            ForEach(v => v.ReplySize += size);
            Delegate?.Reply(size);
        }
    }

    internal class InvocationObserverI : ObserverWithDelegate<InvocationMetrics, IInvocationObserver>,
        IInvocationObserver
    {
        public void RemoteException()
        {
            ForEach(RemoteException);
            Delegate?.RemoteException();
        }

        public void Retried()
        {
            ForEach(IncrementRetry);
            Delegate?.Retried();
        }

        public IRemoteObserver? GetRemoteObserver(ConnectionInfo con, Endpoint endpt, int requestId, int size) =>
            GetObserver<RemoteMetrics, RemoteObserverI, IRemoteObserver>(
                "Remote",
                new RemoteInvocationHelper(con, endpt, requestId, size),
                Delegate?.GetRemoteObserver(con, endpt, requestId, size));

        public ICollocatedObserver? GetCollocatedObserver(ObjectAdapter adapter, int requestId, int size) =>
            GetObserver<CollocatedMetrics, CollocatedObserverI, ICollocatedObserver>(
                "Collocated",
                new CollocatedInvocationHelper(adapter, requestId, size),
                Delegate?.GetCollocatedObserver(adapter, requestId, size));

        private void IncrementRetry(InvocationMetrics v) => ++v.Retry;

        private void RemoteException(InvocationMetrics v) => ++v.UserException;
    }

    public class CommunicatorObserverI : ICommunicatorObserver
    {
        public CommunicatorObserverI(Communicator communicator, ILogger logger)
        {
            _metrics = new MetricsAdminI(communicator, logger);
            _delegate = communicator.Observer;
            _connections = new ObserverFactoryWithDelegate<ConnectionMetrics, ConnectionObserverI,
                IConnectionObserver>(_metrics, "Connection");
            _dispatch = new ObserverFactoryWithDelegate<DispatchMetrics, DispatchObserverI, IDispatchObserver>(
                _metrics, "Dispatch");
            _invocations = new ObserverFactoryWithDelegate<InvocationMetrics, InvocationObserverI,
                IInvocationObserver>(_metrics, "Invocation");
            _connects = new ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
                IObserver>(_metrics, "ConnectionEstablishment");
            _endpointLookups = new ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
                IObserver>(_metrics, "EndpointLookup");
            _invocations.RegisterSubMap<RemoteMetrics>("Remote",
                (obj, metrics) => (obj as InvocationMetrics)!.Remotes = metrics);
            _invocations.RegisterSubMap<CollocatedMetrics>("Collocated",
                (obj, metrics) => (obj as InvocationMetrics)!.Collocated = metrics);
        }

        public IObserver? GetConnectionEstablishmentObserver(Endpoint endpt, string connector)
        {
            if (_connects.IsEnabled())
            {
                try
                {
                    return _connects.GetObserver(new EndpointHelper(endpt, connector),
                        _delegate?.GetConnectionEstablishmentObserver(endpt, connector));
                }
                catch (Exception ex)
                {
                    _metrics.GetLogger().Error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public IObserver? GetEndpointLookupObserver(Endpoint endpt)
        {
            if (_endpointLookups.IsEnabled())
            {
                try
                {
                    return _endpointLookups.GetObserver(new EndpointHelper(endpt),
                        _delegate?.GetEndpointLookupObserver(endpt));
                }
                catch (Exception ex)
                {
                    _metrics.GetLogger().Error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public IConnectionObserver? GetConnectionObserver(ConnectionInfo c,
                                                          Endpoint e,
                                                          ConnectionState s,
                                                          IConnectionObserver? obsv)
        {
            if (_connections.IsEnabled())
            {
                try
                {
                    return _connections.GetObserver(new ConnectionHelper(c, e, s), obsv,
                        _delegate?.GetConnectionObserver(c, e, s, (obsv as ConnectionObserverI)?.Delegate ?? obsv));
                }
                catch (Exception ex)
                {
                    _metrics.GetLogger().Error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public IInvocationObserver? GetInvocationObserver(
            IObjectPrx prx,
            string operation,
            IReadOnlyDictionary<string, string> ctx)
        {
            if (_invocations.IsEnabled())
            {
                try
                {
                    return _invocations.GetObserver(new InvocationHelper(prx, operation, ctx),
                        _delegate?.GetInvocationObserver(prx, operation, ctx));
                }
                catch (Exception ex)
                {
                    _metrics.GetLogger().Error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public IDispatchObserver? GetDispatchObserver(Current current, int size)
        {
            if (_dispatch.IsEnabled())
            {
                try
                {
                    return _dispatch.GetObserver(new DispatchHelper(current, size),
                        _delegate?.GetDispatchObserver(current, size));
                }
                catch (Exception ex)
                {
                    _metrics.GetLogger().Error("unexpected exception trying to obtain observer:\n" + ex);
                }
            }
            return null;
        }

        public void SetObserverUpdater(IObserverUpdater? updater)
        {
            if (updater == null)
            {
                _connections.SetUpdater(null);
            }
            else
            {
                _connections.SetUpdater(updater.UpdateConnectionObservers);
            }

            _delegate?.SetObserverUpdater(updater);
        }

        public MetricsAdminI GetFacet() => _metrics;

        private readonly MetricsAdminI _metrics;
        private readonly ICommunicatorObserver? _delegate;
        private readonly ObserverFactoryWithDelegate<ConnectionMetrics, ConnectionObserverI,
            IConnectionObserver> _connections;
        private readonly ObserverFactoryWithDelegate<DispatchMetrics, DispatchObserverI,
            IDispatchObserver> _dispatch;
        private readonly ObserverFactoryWithDelegate<InvocationMetrics, InvocationObserverI,
            IInvocationObserver> _invocations;
        private readonly ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
            IObserver> _connects;
        private readonly ObserverFactoryWithDelegate<Metrics, ObserverWithDelegateI,
            IObserver> _endpointLookups;
    }
}
