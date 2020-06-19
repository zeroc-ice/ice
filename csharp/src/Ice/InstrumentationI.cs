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
    internal class CollocatedInvocationHelper : MetricsHelper<CollocatedMetrics>
    {
        private string Id { get; }

        private int RequestId { get; }

        private static readonly AttributeResolver _attributes = new AttributeResolverI();
        private readonly int _size;

        internal CollocatedInvocationHelper(ObjectAdapter adapter, int requestId, int size)
            : base(_attributes)
        {
            Id = adapter.Name;
            RequestId = requestId;
            _size = size;
        }
        public override void InitMetrics(CollocatedMetrics v) => v.Size += _size;
        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                Add("parent", obj => "Communicator");
                Add("id", obj => (obj as CollocatedInvocationHelper)?.Id);
                Add("requestId", obj => (obj as CollocatedInvocationHelper)?.RequestId);
            }
        }
    }

    internal class CollocatedObserver : ObserverWithDelegate<CollocatedMetrics, ICollocatedObserver>,
        ICollocatedObserver
    {
        public void Reply(int size)
        {
            ForEach(v => v.ReplySize += size);
            Delegate?.Reply(size);
        }
    }

    internal class CommunicatorObserver : ICommunicatorObserver
    {
        private readonly ObserverFactoryWithDelegate<ConnectionMetrics, ConnectionObserver,
            IConnectionObserver> _connections;
        private readonly ObserverFactoryWithDelegate<Metrics, ObserverWithDelegate,
            IObserver> _connects;
        private readonly ICommunicatorObserver? _delegate;
        private readonly ObserverFactoryWithDelegate<DispatchMetrics, DispatchObserver,
            IDispatchObserver> _dispatch;
        private readonly ObserverFactoryWithDelegate<Metrics, ObserverWithDelegate,
            IObserver> _endpointLookups;
        private readonly ObserverFactoryWithDelegate<InvocationMetrics, InvocationObserver,
            IInvocationObserver> _invocations;
        private readonly MetricsAdminI _metrics;

        internal CommunicatorObserver(Communicator communicator, ILogger logger)
        {
            _metrics = new MetricsAdminI(communicator, logger);
            _delegate = communicator.Observer;
            _connections = new ObserverFactoryWithDelegate<ConnectionMetrics, ConnectionObserver,
                IConnectionObserver>(_metrics, "Connection");
            _dispatch = new ObserverFactoryWithDelegate<DispatchMetrics, DispatchObserver, IDispatchObserver>(
                _metrics, "Dispatch");
            _invocations = new ObserverFactoryWithDelegate<InvocationMetrics, InvocationObserver,
                IInvocationObserver>(_metrics, "Invocation");
            _connects = new ObserverFactoryWithDelegate<Metrics, ObserverWithDelegate,
                IObserver>(_metrics, "ConnectionEstablishment");
            _endpointLookups = new ObserverFactoryWithDelegate<Metrics, ObserverWithDelegate,
                IObserver>(_metrics, "EndpointLookup");
            _invocations.RegisterSubMap<RemoteMetrics>("Remote",
                (obj, metrics) => (obj as InvocationMetrics)!.Remotes = metrics);
            _invocations.RegisterSubMap<CollocatedMetrics>("Collocated",
                (obj, metrics) => (obj as InvocationMetrics)!.Collocated = metrics);
        }

        public IObserver? GetConnectionEstablishmentObserver(Endpoint endpt, string connector)
        {
            if (_connects.IsEnabled)
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

        public IConnectionObserver? GetConnectionObserver(
            Connection c,
            Endpoint e,
            ConnectionState s,
            IConnectionObserver? obsv)
        {
            if (_connections.IsEnabled)
            {
                try
                {
                    return _connections.GetObserver(new ConnectionHelper(c, e, s), obsv,
                        _delegate?.GetConnectionObserver(c, e, s, (obsv as ConnectionObserver)?.Delegate ?? obsv));
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
            if (_dispatch.IsEnabled)
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

        public IObserver? GetEndpointLookupObserver(Endpoint endpt)
        {
            if (_endpointLookups.IsEnabled)
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

        public MetricsAdminI GetFacet() => _metrics;

        public IInvocationObserver? GetInvocationObserver(
            IObjectPrx prx,
            string operation,
            IReadOnlyDictionary<string, string> ctx)
        {
            if (_invocations.IsEnabled)
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
    }

    internal class ConnectionHelper : MetricsHelper<ConnectionMetrics>
    {
        private Connection Connection { get; }

        private Endpoint Endpoint { get; }
        private string Id
        {
            get
            {
                if (_id == null)
                {
                    var os = new StringBuilder();
                    string? localAddres = (Connection as IPConnection)?.LocalAddress?.ToString();
                    string? remoteAddress = (Connection as IPConnection)?.LocalAddress?.ToString();
                    if (localAddres != null)
                    {
                        os.Append(localAddres);
                        if (remoteAddress != null)
                        {
                            os.Append(" -> ");
                            os.Append(remoteAddress);
                        }
                    }
                    else
                    {
                        os.Append("connection-").Append(ConnectionInfo);
                    }
                    if (Connection.Endpoint.ConnectionId.Length > 0)
                    {
                        os.Append(" [").Append(Connection.Endpoint.ConnectionId).Append("]");
                    }
                    _id = os.ToString();
                }
                return _id;
            }
        }

        public string Parent =>
            string.IsNullOrEmpty(Connection.Adapter?.Name) ? "Communicator" : Connection.Adapter.Name;

        public ConnectionState State { get; }

        private static readonly AttributeResolver _attributes = new AttributeResolverI();

        private string? _id;

        internal ConnectionHelper(Connection con, Endpoint endpt, ConnectionState state)
            : base(_attributes)
        {
            Connection = con;
            Endpoint = endpt;
            State = state;
        }

        private class AttributeResolverI : AttributeResolver
        {
            internal AttributeResolverI()
            {
                Add("parent", obj => (obj as ConnectionHelper)?.Parent);
                Add("id", obj => (obj as ConnectionHelper)?.Id);
                Add("state", obj => (obj as ConnectionHelper)?.State.ToString().ToLowerInvariant());
                Add("incoming", obj => (obj as ConnectionHelper)?.Connection.IsIncoming);
                Add("adapterName", obj => (obj as ConnectionHelper)?.Connection.Adapter?.Name);
                Add("connectionId", obj => (obj as ConnectionHelper)?.Connection.Endpoint.ConnectionId);

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
    }

    internal class ConnectionObserver
        : ObserverWithDelegate<ConnectionMetrics, IConnectionObserver>, IConnectionObserver
    {
        private int _receivedBytes;
        private int _sentBytes;

        public void ReceivedBytes(int num)
        {
            _receivedBytes = num;
            ForEach(ReceivedBytesUpdate);
            Delegate?.ReceivedBytes(num);
        }

        public void SentBytes(int num)
        {
            _sentBytes = num;
            ForEach(SentBytesUpdate);
            Delegate?.SentBytes(num);
        }

        private void ReceivedBytesUpdate(ConnectionMetrics v) => v.ReceivedBytes += _receivedBytes;
        private void SentBytesUpdate(ConnectionMetrics v) => v.SentBytes += _sentBytes;
    }

    internal class DispatchHelper : MetricsHelper<DispatchMetrics>
    {
        // It is important to throw here when there isn't a connection, so that the filters doesn't use the
        // connection attributes for a collocated dispatch.
        private Connection Connection => Current.Connection ?? throw new NotSupportedException();
        private Current Current { get; }

        private string Id
        {
            get
            {
                _id ??= $"{Current.Identity} [{Current.Operation}]";
                return _id;
            }
        }

        private string Identity => Current.Identity.ToString(Current.Adapter!.Communicator.ToStringMode);

        private string Mode => Current.RequestId == 0 ? "oneway" : "twoway";

        private string Parent => Current.Adapter.Name;

        private static readonly AttributeResolver _attributes = new AttributeResolverI();
        private readonly int _size;
        private string? _id;

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

        private class AttributeResolverI : AttributeResolver
        {
            internal AttributeResolverI()
            {
                Add("parent", obj => (obj as DispatchHelper)?.Parent);
                Add("id", obj => (obj as DispatchHelper)?.Id);

                Add("incoming", obj => (obj as DispatchHelper)?.Connection.IsIncoming);
                Add("adapterName", obj => (obj as DispatchHelper)?.Current.Adapter.Name);
                Add("connectionId", obj => (obj as DispatchHelper)?.Connection.Endpoint.ConnectionId);

                Add("localHost", obj => (obj as DispatchHelper)?.IPConnectionInfo?.LocalAddress);
                Add("localPort", obj => (obj as DispatchHelper)?.IPConnectionInfo?.LocalPort);
                Add("remoteHost", obj => (obj as DispatchHelper)?.IPConnectionInfo?.RemoteAddress);
                Add("remotePort", obj => (obj as DispatchHelper)?.IPConnectionInfo?.RemotePort);

                Add("mcastHost",
                    obj => ((obj as DispatchHelper)?.ConnectionInfo as UDPConnectionInfo)?.McastAddress);
                Add("mcastPort",
                    obj => ((obj as DispatchHelper)?.ConnectionInfo as UDPConnectionInfo)?.McastPort);

                Add("endpoint", obj => (obj as DispatchHelper)?.Connection.Endpoint);
                Add("endpointType", obj => (obj as DispatchHelper)?.Connection.Endpoint.Type);
                Add("endpointIsDatagram", obj => (obj as DispatchHelper)?.Connection.Endpoint.IsDatagram);
                Add("endpointIsSecure", obj => (obj as DispatchHelper)?.Connection.Endpoint.IsSecure);
                Add("endpointTimeout", obj => (obj as DispatchHelper)?.Connection.Endpoint.Timeout);
                Add("endpointCompress", obj => (obj as DispatchHelper)?.Connection.Endpoint.HasCompressionFlag);
                Add("endpointHost", obj => ((obj as DispatchHelper)?.Connection.Endpoint as IPEndpoint)?.Host);
                Add("endpointPort", obj => ((obj as DispatchHelper)?.Connection.Endpoint as IPEndpoint)?.Port);

                Add("operation", obj => (obj as DispatchHelper)?.Current.Operation);
                Add("identity", obj => (obj as DispatchHelper)?.Identity);
                Add("facet", obj => (obj as DispatchHelper)?.Current.Facet);
                Add("requestId", obj => (obj as DispatchHelper)?.Current.RequestId);
                Add("mode", obj => (obj as DispatchHelper)?.Mode);
            }
        }
    }

    internal class DispatchObserver : ObserverWithDelegate<DispatchMetrics, IDispatchObserver>, IDispatchObserver
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

    internal class EndpointHelper : MetricsHelper<Metrics>
    {
        private Endpoint Endpoint { get; }

        private string Id
        {
            get
            {
                _id ??= Endpoint.ToString();
                return _id;
            }
        }

        private static readonly AttributeResolver _attributes = new AttributeResolverI();
        private string? _id;

        internal EndpointHelper(Endpoint endpt, string id) : base(_attributes)
        {
            Endpoint = endpt;
            _id = id;
        }

        internal EndpointHelper(Endpoint endpt) : base(_attributes) => Endpoint = endpt;

        private class AttributeResolverI : AttributeResolver
        {
            internal AttributeResolverI()
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
    }

    internal class InvocationHelper : MetricsHelper<InvocationMetrics>
    {
        private string Id
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

        private string Identity => Proxy.Identity.ToString(Proxy.Communicator.ToStringMode);

        private string Mode => Proxy.InvocationMode.ToString().ToLowerInvariant();

        private string Operation { get; }

        private string Parent => "Communicator";

        private IObjectPrx Proxy { get; }

        private static readonly AttributeResolver _attributes = new AttributeResolverI();
        private readonly IReadOnlyDictionary<string, string> _context;
        private string? _id;

        internal InvocationHelper(IObjectPrx proxy, string op, IReadOnlyDictionary<string, string> ctx)
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

        private class AttributeResolverI : AttributeResolver
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
    }

    internal class InvocationObserver : ObserverWithDelegate<InvocationMetrics, IInvocationObserver>,
        IInvocationObserver
    {
        public ICollocatedObserver? GetCollocatedObserver(ObjectAdapter adapter, int requestId, int size) =>
            GetObserver<CollocatedMetrics, CollocatedObserver, ICollocatedObserver>(
                "Collocated",
                new CollocatedInvocationHelper(adapter, requestId, size),
                Delegate?.GetCollocatedObserver(adapter, requestId, size));

        public IRemoteObserver? GetRemoteObserver(Connection con, Endpoint endpt, int requestId, int size) =>
            GetObserver<RemoteMetrics, RemoteObserver, IRemoteObserver>(
                "Remote",
                new RemoteInvocationHelper(con, endpt, requestId, size),
                Delegate?.GetRemoteObserver(con, endpt, requestId, size));

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

        private void IncrementRetry(InvocationMetrics v) => ++v.Retry;

        private void RemoteException(InvocationMetrics v) => ++v.UserException;
    }

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

    internal class ObserverWithDelegate : ObserverWithDelegate<Metrics, IObserver>
    {
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

    internal class RemoteInvocationHelper : MetricsHelper<RemoteMetrics>
    {
        private Connection Connection { get; }
        private Endpoint Endpoint { get; }
        private string Id
        {
            get
            {
                if (_id == null)
                {
                    _id = string.IsNullOrEmpty(Connection.Endpoint.ConnectionId) ?
                        Endpoint.ToString() : $"{Endpoint} [" + Connection.Endpoint.ConnectionId + "]";
                }
                return _id;
            }
        }

        private string Parent =>
            string.IsNullOrEmpty(Connection.Adapter?.Name) ? "Communicator" : Connection.Adapter.Name;

        private int RequestId { get; }

        private static readonly AttributeResolver _attributes = new AttributeResolverI();
        private readonly int _size;
        private string? _id;

        internal RemoteInvocationHelper(Connection con, Endpoint endpt, int requestId, int size)
            : base(_attributes)
        {
            Connection = con;
            Endpoint = endpt;
            RequestId = requestId;
            _size = size;
        }

        public override void InitMetrics(RemoteMetrics v) => v.Size += _size;

        private class AttributeResolverI : AttributeResolver
        {
            internal AttributeResolverI()
            {
                Add("parent", obj => (obj as RemoteInvocationHelper)?.Parent);
                Add("id", obj => (obj as RemoteInvocationHelper)?.Id);
                Add("requestId", obj => (obj as RemoteInvocationHelper)?.RequestId);
                Add("incoming", obj => (obj as RemoteInvocationHelper)?.Connection.IsIncoming);
                Add("adapterName", obj => (obj as RemoteInvocationHelper)?.Connection.Adapter?.Name);
                Add("connectionId", obj => (obj as RemoteInvocationHelper)?.Connection.Endpoint.ConnectionId);

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
    }

    internal class RemoteObserver : ObserverWithDelegate<RemoteMetrics, IRemoteObserver>, IRemoteObserver
    {
        public void Reply(int size)
        {
            ForEach(v => v.ReplySize += size);
            Delegate?.Reply(size);
        }
    }
}
