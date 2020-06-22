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
        private static readonly AttributeResolver _attributeResolver = new AttributeResolverI();
        private readonly int _requestId;
        private readonly string _id;
        private readonly int _size;

        public override void InitMetrics(CollocatedMetrics v) => v.Size += _size;

        internal CollocatedInvocationHelper(ObjectAdapter adapter, int requestId, int size)
            : base(_attributeResolver)
        {
            _id = adapter.Name;
            _requestId = requestId;
            _size = size;
        }

        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                Add("parent", obj => "Communicator");
                Add("id", obj => (obj as CollocatedInvocationHelper)?._id);
                Add("requestId", obj => (obj as CollocatedInvocationHelper)?._requestId);
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

        internal CommunicatorObserver(Communicator communicator, ILogger logger)
        {
            AdminFacet = new MetricsAdmin(communicator, logger);
            _delegate = communicator.Observer;
            _connections = new ObserverFactoryWithDelegate<ConnectionMetrics, ConnectionObserver,
                IConnectionObserver>(AdminFacet, "Connection");
            _dispatch = new ObserverFactoryWithDelegate<DispatchMetrics, DispatchObserver, IDispatchObserver>(
                AdminFacet, "Dispatch");
            _invocations = new ObserverFactoryWithDelegate<InvocationMetrics, InvocationObserver,
                IInvocationObserver>(AdminFacet, "Invocation");
            _connects = new ObserverFactoryWithDelegate<Metrics, ObserverWithDelegate,
                IObserver>(AdminFacet, "ConnectionEstablishment");
            _endpointLookups = new ObserverFactoryWithDelegate<Metrics, ObserverWithDelegate,
                IObserver>(AdminFacet, "EndpointLookup");
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
                    AdminFacet.Logger.Error($"unexpected exception trying to obtain observer:\n{ex}");
                }
            }
            return null;
        }

        public IConnectionObserver? GetConnectionObserver(
            ConnectionInfo connectionInfo,
            Endpoint endpoint,
            ConnectionState connectionState,
            IConnectionObserver? observer)
        {
            if (_connections.IsEnabled)
            {
                try
                {
                    return _connections.GetObserver(
                        new ConnectionHelper(connectionInfo, endpoint, connectionState), observer,
                        _delegate?.GetConnectionObserver(connectionInfo, endpoint, connectionState,
                            (observer as ConnectionObserver)?.Delegate ?? observer));
                }
                catch (Exception ex)
                {
                    AdminFacet.Logger.Error($"unexpected exception trying to obtain observer:\n{ex}");
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
                    AdminFacet.Logger.Error($"unexpected exception trying to obtain observer:\n{ex}");
                }
            }
            return null;
        }

        public IObserver? GetEndpointLookupObserver(Endpoint endpoint)
        {
            if (_endpointLookups.IsEnabled)
            {
                try
                {
                    return _endpointLookups.GetObserver(new EndpointHelper(endpoint),
                        _delegate?.GetEndpointLookupObserver(endpoint));
                }
                catch (Exception ex)
                {
                    AdminFacet.Logger.Error($"unexpected exception trying to obtain observer:\n{ex}");
                }
            }
            return null;
        }

        public MetricsAdmin AdminFacet { get; }

        public IInvocationObserver? GetInvocationObserver(
            IObjectPrx prx,
            string operation,
            IReadOnlyDictionary<string, string> context)
        {
            if (_invocations.IsEnabled)
            {
                try
                {
                    return _invocations.GetObserver(new InvocationHelper(prx, operation, context),
                        _delegate?.GetInvocationObserver(prx, operation, context));
                }
                catch (Exception ex)
                {
                    AdminFacet.Logger.Error($"unexpected exception trying to obtain observer:\n{ex}");
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
        private string Id
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
        }

        // TODO temporary until ConnectionInfo refactoring
        private IPConnectionInfo? IPConnectionInfo
        {
            get
            {
                for (ConnectionInfo? p = _connectionInfo; p != null; p = p.Underlying)
                {
                    if (p is IPConnectionInfo info)
                    {
                        return info;
                    }
                }
                return null;
            }
        }

        private static readonly AttributeResolver _attributeResolver = new AttributeResolverI();

        private readonly ConnectionInfo _connectionInfo;
        private readonly Endpoint _endpoint;
        private string? _id;
        private readonly string _parent;
        private readonly ConnectionState _state;

        internal ConnectionHelper(ConnectionInfo connectionInfo, Endpoint endpoint, ConnectionState state)
            : base(_attributeResolver)
        {
            _connectionInfo = connectionInfo;
            _endpoint = endpoint;
            _parent = string.IsNullOrEmpty(_connectionInfo.AdapterName) ? "Communicator" : _connectionInfo.AdapterName;
            _state = state;
        }

        private class AttributeResolverI : AttributeResolver
        {
            internal AttributeResolverI()
            {
                Add("parent", obj => (obj as ConnectionHelper)?._parent);
                Add("id", obj => (obj as ConnectionHelper)?.Id);
                Add("state", obj => (obj as ConnectionHelper)?._state.ToString().ToLowerInvariant());
                Add("incoming", obj => (obj as ConnectionHelper)?._connectionInfo.Incoming);
                Add("adapterName", obj => (obj as ConnectionHelper)?._connectionInfo.AdapterName);
                Add("connectionId", obj => (obj as ConnectionHelper)?._connectionInfo.ConnectionId);

                Add("localHost", obj => (obj as ConnectionHelper)?.IPConnectionInfo?.LocalAddress);
                Add("localPort", obj => (obj as ConnectionHelper)?.IPConnectionInfo?.LocalPort);
                Add("remoteHost", obj => (obj as ConnectionHelper)?.IPConnectionInfo?.RemoteAddress);
                Add("remotePort", obj => (obj as ConnectionHelper)?.IPConnectionInfo?.RemotePort);

                Add("mcastHost",
                    obj => ((obj as ConnectionHelper)?._connectionInfo as UDPConnectionInfo)?.McastAddress);
                Add("mcastPort",
                    obj => ((obj as ConnectionHelper)?._connectionInfo as UDPConnectionInfo)?.McastPort);

                Add("endpoint", obj => (obj as ConnectionHelper)?._endpoint);
                Add("endpointTransport", obj => (obj as ConnectionHelper)?._endpoint?.Transport);
                Add("endpointIsDatagram", obj => (obj as ConnectionHelper)?._endpoint?.IsDatagram);
                Add("endpointIsSecure", obj => (obj as ConnectionHelper)?._endpoint?.IsSecure);
                Add("endpointTimeout", obj => (obj as ConnectionHelper)?._endpoint?.Timeout);
                Add("endpointCompress", obj => (obj as ConnectionHelper)?._endpoint?.HasCompressionFlag);
                Add("endpointHost", obj => ((obj as ConnectionHelper)?._endpoint as IPEndpoint)?.Host);
                Add("endpointPort", obj => ((obj as ConnectionHelper)?._endpoint as IPEndpoint)?.Port);
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
            ForEach(v => v.ReceivedBytes += _receivedBytes);
            Delegate?.ReceivedBytes(num);
        }

        public void SentBytes(int num)
        {
            _sentBytes = num;
            ForEach(v => v.SentBytes += _sentBytes);
            Delegate?.SentBytes(num);
        }
    }

    internal class DispatchHelper : MetricsHelper<DispatchMetrics>
    {
        // It is important to throw here when there isn't a connection, so that the filters doesn't use the
        // connection attributes for a collocated dispatch.
        private ConnectionInfo? ConnectionInfo => _current.Connection?.GetConnectionInfo() ??
            throw new NotSupportedException();

        // It is important to throw here when there isn't a connection, so that the filters doesn't use the
        // connection attributes for a collocated dispatch.
        private Endpoint? Endpoint => _current.Connection?.Endpoint ?? throw new NotSupportedException();

        private string Id
        {
            get
            {
                _id ??= $"{_current.Identity} [{_current.Operation}]";
                return _id;
            }
        }

        private string Identity => _current.Identity.ToString(_current.Adapter!.Communicator.ToStringMode);

        // TODO temporary until ConnectionInfo refactoring
        private IPConnectionInfo? IPConnectionInfo
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

        private static readonly AttributeResolver _attributeResolver = new AttributeResolverI();

        private readonly Current _current;
        private string? _id;
        private readonly string _mode;
        private readonly string _parent;
        private readonly int _size;

        public override void InitMetrics(DispatchMetrics v) => v.Size += _size;

        internal DispatchHelper(Current current, int size)
            : base(_attributeResolver)
        {
            _current = current;
            _mode = _current.RequestId == 0 ? "oneway" : "twoway";
            _parent = _current.Adapter.Name;
            _size = size;
        }

        protected override string DefaultResolve(string attribute)
        {
            if (attribute.StartsWith("context.") && _current.Context.TryGetValue(attribute.Substring(8), out string? v))
            {
                return v;
            }
            throw new ArgumentOutOfRangeException(attribute);
        }

        private class AttributeResolverI : AttributeResolver
        {
            internal AttributeResolverI()
            {
                Add("parent", obj => (obj as DispatchHelper)?._parent);
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
                Add("endpointTransport", obj => (obj as DispatchHelper)?.Endpoint?.Transport);
                Add("endpointIsDatagram", obj => (obj as DispatchHelper)?.Endpoint?.IsDatagram);
                Add("endpointIsSecure", obj => (obj as DispatchHelper)?.Endpoint?.IsSecure);
                Add("endpointTimeout", obj => (obj as DispatchHelper)?.Endpoint?.Timeout);
                Add("endpointCompress", obj => (obj as DispatchHelper)?.Endpoint?.HasCompressionFlag);
                Add("endpointHost", obj => ((obj as DispatchHelper)?.Endpoint as IPEndpoint)?.Host);
                Add("endpointPort", obj => ((obj as DispatchHelper)?.Endpoint as IPEndpoint)?.Port);

                Add("operation", obj => (obj as DispatchHelper)?._current.Operation);
                Add("identity", obj => (obj as DispatchHelper)?.Identity);
                Add("facet", obj => (obj as DispatchHelper)?._current.Facet);
                Add("requestId", obj => (obj as DispatchHelper)?._current.RequestId);
                Add("mode", obj => (obj as DispatchHelper)?._mode);
            }
        }
    }

    internal class DispatchObserver : ObserverWithDelegate<DispatchMetrics, IDispatchObserver>, IDispatchObserver
    {
        public void RemoteException()
        {
            ForEach(v => ++v.UserException);
            Delegate?.RemoteException();
        }

        public void Reply(int size)
        {
            ForEach(v => v.ReplySize += size);
            Delegate?.Reply(size);
        }
    }

    internal class EndpointHelper : MetricsHelper<Metrics>
    {
        private string Id
        {
            get
            {
                _id ??= _endpoint.ToString();
                return _id;
            }
        }

        private static readonly AttributeResolver _attributeResolver = new AttributeResolverI();

        private readonly Endpoint _endpoint;
        private string? _id;

        internal EndpointHelper(Endpoint endpoint, string id) : base(_attributeResolver)
        {
            _endpoint = endpoint;
            _id = id;
        }

        internal EndpointHelper(Endpoint endpoint) : base(_attributeResolver) => _endpoint = endpoint;

        private class AttributeResolverI : AttributeResolver
        {
            internal AttributeResolverI()
            {
                Add("parent", obj => "Communicator");
                Add("id", obj => (obj as EndpointHelper)?.Id);
                Add("endpoint", obj => (obj as EndpointHelper)?._endpoint);
                Add("endpointTransport", obj => (obj as EndpointHelper)?._endpoint?.Transport);
                Add("endpointIsDatagram", obj => (obj as EndpointHelper)?._endpoint?.IsDatagram);
                Add("endpointIsSecure", obj => (obj as EndpointHelper)?._endpoint?.IsSecure);
                Add("endpointTimeout", obj => (obj as EndpointHelper)?._endpoint?.Timeout);
                Add("endpointCompress", obj => (obj as EndpointHelper)?._endpoint?.HasCompressionFlag);
                Add("endpointHost", obj => ((obj as EndpointHelper)?._endpoint as IPEndpoint)?.Host);
                Add("endpointPort", obj => ((obj as EndpointHelper)?._endpoint as IPEndpoint)?.Port);
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
                        sb.Append(_proxy.Clone(endpoints: Array.Empty<Endpoint>()));
                        sb.Append(" [").Append(_operation).Append(']');
                    }
                    catch (Exception)
                    {
                        // Either a fixed proxy or the communicator is destroyed.
                        sb.Append(_proxy.Identity.ToString(_proxy.Communicator.ToStringMode));
                        sb.Append(" [").Append(_operation).Append(']');
                    }
                    _id = sb.ToString();
                }
                return _id;
            }
        }

        private string Identity => _proxy.Identity.ToString(_proxy.Communicator.ToStringMode);

        private static readonly AttributeResolver _attributeResolver = new AttributeResolverI();
        private readonly IReadOnlyDictionary<string, string> _context;
        private string? _id;
        private readonly string _operation;
        private readonly IObjectPrx _proxy;

        internal InvocationHelper(IObjectPrx proxy, string operation, IReadOnlyDictionary<string, string> context)
            : base(_attributeResolver)
        {
            _proxy = proxy;
            _operation = operation;
            _context = context;
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
                Add("parent", obj => "Communicator");
                Add("id", obj => (obj as InvocationHelper)?.Id);

                Add("operation", obj => (obj as InvocationHelper)?._operation);
                Add("identity", obj => (obj as InvocationHelper)?.Identity);

                Add("facet", obj => (obj as InvocationHelper)?._proxy.Facet);
                Add("encoding", obj => (obj as InvocationHelper)?._proxy.Encoding);
                Add("mode", obj => (obj as InvocationHelper)?._proxy.InvocationMode.ToString().ToLowerInvariant());
                Add("proxy", obj => (obj as InvocationHelper)?._proxy);
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

        public IRemoteObserver? GetRemoteObserver(
            ConnectionInfo connectionInfo,
            Endpoint endpoint,
            int requestId,
            int size) =>
            GetObserver<RemoteMetrics, RemoteObserver, IRemoteObserver>(
                "Remote",
                new RemoteInvocationHelper(connectionInfo, endpoint, requestId, size),
                Delegate?.GetRemoteObserver(connectionInfo, endpoint, requestId, size));

        public void RemoteException()
        {
            ForEach(v => ++v.UserException);
            Delegate?.RemoteException();
        }

        public void Retried()
        {
            ForEach(v => ++v.Retry);
            Delegate?.Retried();
        }
    }

    internal class ObserverFactoryWithDelegate<T, OImpl, O> : ObserverFactory<T, OImpl>
        where T : Metrics, new()
        where OImpl : ObserverWithDelegate<T, O>, O, new()
        where O : class, IObserver
    {
        public ObserverFactoryWithDelegate(MetricsAdmin metrics, string name)
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

    internal class RemoteInvocationHelper : MetricsHelper<RemoteMetrics>
    {
        private string Id
        {
            get
            {
                if (_id == null)
                {
                    _id = string.IsNullOrEmpty(_connectionInfo.ConnectionId) ?
                        _endpoint.ToString() : $"{_endpoint} [" + _connectionInfo.ConnectionId + "]";
                }
                return _id;
            }
        }

        // TODO temporary until ConnectionInfo refactoring
        private IPConnectionInfo? IPConnectionInfo
        {
            get
            {
                for (ConnectionInfo? p = _connectionInfo; p != null; p = p.Underlying)
                {
                    if (p is IPConnectionInfo info)
                    {
                        return info;
                    }
                }
                return null;
            }
        }

        private static readonly AttributeResolver _attributeResolver = new AttributeResolverI();

        private readonly ConnectionInfo _connectionInfo;
        private readonly Endpoint _endpoint;
        private readonly string _parent;
        private readonly int _requestId;
        private readonly int _size;
        private string? _id;

        public override void InitMetrics(RemoteMetrics v) => v.Size += _size;

        internal RemoteInvocationHelper(ConnectionInfo connectionInfo, Endpoint endpoint, int requestId, int size)
            : base(_attributeResolver)
        {
            _connectionInfo = connectionInfo;
            _endpoint = endpoint;
            _parent = string.IsNullOrEmpty(_connectionInfo.AdapterName) ? "Communicator" : _connectionInfo.AdapterName;
            _requestId = requestId;
            _size = size;
        }

        private class AttributeResolverI : AttributeResolver
        {
            internal AttributeResolverI()
            {
                Add("parent", obj => (obj as RemoteInvocationHelper)?._parent);
                Add("id", obj => (obj as RemoteInvocationHelper)?.Id);
                Add("requestId", obj => (obj as RemoteInvocationHelper)?._requestId);
                Add("incoming", obj => (obj as RemoteInvocationHelper)?._connectionInfo.Incoming);
                Add("adapterName", obj => (obj as RemoteInvocationHelper)?._connectionInfo.AdapterName);
                Add("connectionId", obj => (obj as RemoteInvocationHelper)?._connectionInfo.ConnectionId);

                Add("localHost", obj => (obj as RemoteInvocationHelper)?.IPConnectionInfo?.LocalAddress);
                Add("localPort", obj => (obj as RemoteInvocationHelper)?.IPConnectionInfo?.LocalPort);
                Add("remoteHost", obj => (obj as RemoteInvocationHelper)?.IPConnectionInfo?.RemoteAddress);
                Add("remotePort", obj => (obj as RemoteInvocationHelper)?.IPConnectionInfo?.RemotePort);

                Add("mcastHost",
                    obj => ((obj as RemoteInvocationHelper)?._connectionInfo as UDPConnectionInfo)?.McastAddress);
                Add("mcastPort",
                    obj => ((obj as RemoteInvocationHelper)?._connectionInfo as UDPConnectionInfo)?.McastPort);
                Add("endpoint", obj => (obj as RemoteInvocationHelper)?._endpoint);
                Add("endpointTransport", obj => (obj as RemoteInvocationHelper)?._endpoint?.Transport);
                Add("endpointIsDatagram", obj => (obj as RemoteInvocationHelper)?._endpoint?.IsDatagram);
                Add("endpointIsSecure", obj => (obj as RemoteInvocationHelper)?._endpoint?.IsSecure);
                Add("endpointTimeout", obj => (obj as RemoteInvocationHelper)?._endpoint?.Timeout);
                Add("endpointCompress", obj => (obj as RemoteInvocationHelper)?._endpoint?.HasCompressionFlag);
                Add("endpointHost", obj => ((obj as RemoteInvocationHelper)?._endpoint as IPEndpoint)?.Host);
                Add("endpointPort", obj => ((obj as RemoteInvocationHelper)?._endpoint as IPEndpoint)?.Port);
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
