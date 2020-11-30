// Copyright (c) ZeroC, Inc. All rights reserved.

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
        /// <summary>Constructs a new InvocationMetrics object.</summary>
        public InvocationMetrics()
            : this(children: Array.Empty<Metrics>(), remotes: Array.Empty<Metrics>(), collocated: Array.Empty<Metrics>())
        {
        }
    }
}

namespace ZeroC.Ice
{
    internal class ChildInvocationObserver :
        ObserverWithDelegate<ChildInvocationMetrics, IChildInvocationObserver>, IChildInvocationObserver
    {
        public void Reply(int size)
        {
            ForEach(v => v.ReplySize += size);
            Delegate?.Reply(size);
        }
    }

    internal class CommunicatorObserver : ICommunicatorObserver
    {
        public MetricsAdmin AdminFacet { get; }

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
            _invocations.RegisterSubMap<ChildInvocationMetrics>("ChildInvocation",
                (obj, metrics) => (obj as InvocationMetrics)!.Children = metrics);
        }

        public IObserver? GetConnectionEstablishmentObserver(Endpoint endpoint, string description)
        {
            if (_connects.IsEnabled)
            {
                try
                {
                    return _connects.GetObserver(new EndpointHelper(endpoint, description),
                        _delegate?.GetConnectionEstablishmentObserver(endpoint, description));
                }
                catch (Exception ex)
                {
                    AdminFacet.Logger.Error($"unexpected exception trying to obtain observer:\n{ex}");
                }
            }
            return null;
        }

        public IConnectionObserver? GetConnectionObserver(
            Connection connection,
            ConnectionState connectionState,
            IConnectionObserver? observer)
        {
            if (_connections.IsEnabled)
            {
                try
                {
                    return _connections.GetObserver(
                        new ConnectionHelper(connection, connectionState),
                        observer,
                        _delegate?.GetConnectionObserver(
                            connection,
                            connectionState,
                            (observer as ConnectionObserver)?.Delegate ?? observer));
                }
                catch (Exception ex)
                {
                    AdminFacet.Logger.Error($"unexpected exception trying to obtain observer:\n{ex}");
                }
            }
            return null;
        }

        public IDispatchObserver? GetDispatchObserver(Current current, long streamId, int size)
        {
            if (_dispatch.IsEnabled)
            {
                try
                {
                    return _dispatch.GetObserver(new DispatchHelper(current, streamId, size),
                        _delegate?.GetDispatchObserver(current, streamId, size));
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
                    if ((_connection as IPConnection)?.LocalEndpoint?.ToString() is string localAddress)
                    {
                        os.Append(localAddress);
                        if ((_connection as IPConnection)?.RemoteEndpoint?.ToString() is string remoteAddress)
                        {
                            os.Append(" -> ");
                            os.Append(remoteAddress);
                        }
                    }

                    if (_connection.Label != null)
                    {
                        os.Append(" [").Append(_connection.Label).Append(']');
                    }
                    _id = os.ToString();
                }
                return _id;
            }
        }

        private static readonly AttributeResolver _attributeResolver = new AttributeResolverI();

        private readonly Connection _connection;
        private string? _id;
        private readonly ConnectionState _state;

        internal ConnectionHelper(Connection connection, ConnectionState state)
            : base(_attributeResolver)
        {
            _connection = connection;
            _state = state;
        }

        private class AttributeResolverI : AttributeResolver
        {
            internal AttributeResolverI()
            {
                Add("parent", obj => (obj as ConnectionHelper)?._connection?.Adapter?.Name ?? "Communicator");
                Add("id", obj => (obj as ConnectionHelper)?.Id);
                Add("state", obj => (obj as ConnectionHelper)?._state.ToString().ToLowerInvariant());
                Add("incoming", obj => (obj as ConnectionHelper)?._connection.IsIncoming);
                Add("adapterName", obj => (obj as ConnectionHelper)?._connection.Adapter?.Name);
                Add("label", obj => (obj as ConnectionHelper)?._connection.Label);

                Add("localHost", obj =>
                    ((obj as ConnectionHelper)?._connection as IPConnection)?.LocalEndpoint?.Address);

                Add("localPort", obj =>
                    ((obj as ConnectionHelper)?._connection as IPConnection)?.LocalEndpoint?.Port);

                Add("remoteHost", obj =>
                    ((obj as ConnectionHelper)?._connection as IPConnection)?.RemoteEndpoint?.Address);

                Add("remotePort", obj =>
                    ((obj as ConnectionHelper)?._connection as IPConnection)?.RemoteEndpoint?.Port);

                Add("mcastHost", obj =>
                    ((obj as ConnectionHelper)?._connection as UdpConnection)?.MulticastEndpoint?.Address);

                Add("mcastPort", obj =>
                    ((obj as ConnectionHelper)?._connection as UdpConnection)?.MulticastEndpoint?.Port);

                Add("endpoint", obj => (obj as ConnectionHelper)?._connection.Endpoint);
                Add("endpointTransport", obj => (obj as ConnectionHelper)?._connection.Endpoint?.Transport);
                Add("endpointIsDatagram", obj => (obj as ConnectionHelper)?._connection.Endpoint?.IsDatagram);
                Add("endpointIsAlwaysSecure", obj => (obj as ConnectionHelper)?._connection.Endpoint?.IsAlwaysSecure);
                Add("endpointHost", obj => (obj as ConnectionHelper)?._connection.Endpoint?.Host);
                Add("endpointPort", obj => (obj as ConnectionHelper)?._connection.Endpoint?.Port);
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
        private Connection Connection => _current.Connection ?? throw new MissingFieldException();

        private static readonly AttributeResolver _attributeResolver = new AttributeResolverI();

        private readonly Current _current;
        private readonly long _streamId;

        private string? _id;
        private readonly int _size;

        public override void InitMetrics(DispatchMetrics v) => v.Size += _size;

        internal DispatchHelper(Current current, long streamId, int size)
            : base(_attributeResolver)
        {
            _current = current;
            _streamId = streamId;
            _size = size;
        }

        protected override string DefaultResolve(string attribute) =>
            attribute.StartsWith("context.", StringComparison.InvariantCulture) &&
            _current.Context.TryGetValue(attribute[8..], out string? v) ?
                v : throw new MissingFieldException(attribute);

        private class AttributeResolverI : AttributeResolver
        {
            internal AttributeResolverI()
            {
                Add("parent", obj => (obj as DispatchHelper)?._current.Adapter.Name);
                Add("id", obj =>
                    {
                        DispatchHelper? helper = (obj as DispatchHelper)!;
                        return helper._id ??= $"{helper._current.Identity} [{helper._current.Operation}]";
                    });

                Add("incoming", obj => (obj as DispatchHelper)?.Connection.IsIncoming);
                Add("adapterName", obj => (obj as DispatchHelper)?.Connection.Adapter?.Name);
                Add("label", obj => (obj as DispatchHelper)?.Connection.Label);

                Add("localHost", obj =>
                    ((obj as DispatchHelper)?.Connection as IPConnection)?.LocalEndpoint?.Address);

                Add("localPort", obj =>
                    ((obj as DispatchHelper)?.Connection as IPConnection)?.LocalEndpoint?.Port);

                Add("remoteHost", obj =>
                    ((obj as DispatchHelper)?.Connection as IPConnection)?.RemoteEndpoint?.Address);

                Add("remotePort", obj =>
                    ((obj as DispatchHelper)?.Connection as IPConnection)?.RemoteEndpoint?.Port);

                Add("mcastHost", obj =>
                    ((obj as DispatchHelper)?.Connection as UdpConnection)?.MulticastEndpoint?.Address);

                Add("mcastPort", obj =>
                    ((obj as DispatchHelper)?.Connection as UdpConnection)?.MulticastEndpoint?.Port);

                Add("endpoint", obj => (obj as DispatchHelper)?.Connection.Endpoint);
                Add("endpointTransport", obj => (obj as DispatchHelper)?.Connection.Endpoint?.Transport);
                Add("endpointIsDatagram", obj => (obj as DispatchHelper)?.Connection.Endpoint?.IsDatagram);
                Add("endpointIsAlwaysSecure", obj => (obj as DispatchHelper)?.Connection.Endpoint?.IsAlwaysSecure);
                Add("endpointHost", obj => (obj as DispatchHelper)?.Connection.Endpoint?.Host);
                Add("endpointPort", obj => (obj as DispatchHelper)?.Connection.Endpoint?.Port);

                Add("operation", obj => (obj as DispatchHelper)?._current.Operation);
                Add("identity", obj =>
                    {
                        Current? current = (obj as DispatchHelper)?._current;
                        return current?.Identity.ToString(current.Adapter!.Communicator.ToStringMode);
                    });
                Add("facet", obj => (obj as DispatchHelper)?._current.Facet);
                Add("streamId", obj => (obj as DispatchHelper)?._streamId);
                Add("mode", obj => (obj as DispatchHelper)?._current.IsOneway == true ? "oneway" : "twoway");
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
        private static readonly AttributeResolver _attributeResolver = new AttributeResolverI();

        private readonly Endpoint _endpoint;
        private string? _id;

        internal EndpointHelper(Endpoint endpoint, string id)
            : base(_attributeResolver)
        {
            _endpoint = endpoint;
            _id = id;
        }

        internal EndpointHelper(Endpoint endpoint)
            : base(_attributeResolver) => _endpoint = endpoint;

        private class AttributeResolverI : AttributeResolver
        {
            internal AttributeResolverI()
            {
                Add("parent", obj => "Communicator");
                Add("id", obj =>
                    {
                        EndpointHelper helper = (obj as EndpointHelper)!;
                        return helper._id ??= helper._endpoint.ToString();
                    });
                Add("endpoint", obj => (obj as EndpointHelper)?._endpoint);
                Add("endpointTransport", obj => (obj as EndpointHelper)?._endpoint?.Transport);
                Add("endpointIsDatagram", obj => (obj as EndpointHelper)?._endpoint?.IsDatagram);
                Add("endpointIsAlwaysSecure", obj => (obj as EndpointHelper)?._endpoint?.IsAlwaysSecure);
                Add("endpointHost", obj => (obj as EndpointHelper)?._endpoint.Host);
                Add("endpointPort", obj => (obj as EndpointHelper)?._endpoint.Port);
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

        protected override string DefaultResolve(string attribute) =>
            attribute.StartsWith("context.", StringComparison.InvariantCulture) &&
            _context.TryGetValue(attribute[8..], out string? v) ?
                v : throw new MissingFieldException(attribute);

        private class AttributeResolverI : AttributeResolver
        {
            public AttributeResolverI()
            {
                Add("parent", obj => "Communicator");
                Add("id", obj => (obj as InvocationHelper)?.Id);

                Add("operation", obj => (obj as InvocationHelper)?._operation);
                Add("identity", obj =>
                {
                    IObjectPrx? proxy = (obj as InvocationHelper)?._proxy;
                    return proxy?.Identity.ToString(proxy.Communicator.ToStringMode);
                });

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
        public IChildInvocationObserver? GetChildInvocationObserver(Connection connection, int size) =>
            GetObserver<ChildInvocationMetrics, ChildInvocationObserver, IChildInvocationObserver>(
                "ChildInvocation",
                new ChildInvocationHelper(connection, size),
                Delegate?.GetChildInvocationObserver(connection, size));

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

    internal class ChildInvocationHelper : MetricsHelper<ChildInvocationMetrics>
    {
        private string Id
        {
            get
            {
                _id ??= _connection.Label == null ?
                    $"{_connection.Endpoint}" : $"{_connection.Endpoint} [{_connection.Label}]";
                return _id;
            }
        }

        private static readonly AttributeResolver _attributeResolver = new AttributeResolverI();

        private readonly Connection _connection;
        private readonly int _size;
        private string? _id;

        public override void InitMetrics(ChildInvocationMetrics v) => v.Size += _size;

        internal ChildInvocationHelper(Connection connection, int size)
            : base(_attributeResolver)
        {
            _connection = connection;
            _size = size;
        }

        private class AttributeResolverI : AttributeResolver
        {
            internal AttributeResolverI()
            {
                Add("parent", obj =>
                    {
                        Connection connection = ((ChildInvocationHelper)obj)._connection;
                        return string.IsNullOrEmpty(connection.Adapter?.Name) ? "Communicator" : connection.Adapter?.Name;
                    });
                Add("id", obj => (obj as ChildInvocationHelper)?.Id);
                Add("incoming", obj => (obj as ChildInvocationHelper)?._connection.IsIncoming);
                Add("adapterName", obj => (obj as ChildInvocationHelper)?._connection.Adapter?.Name);
                Add("label", obj => (obj as ChildInvocationHelper)?._connection.Label);

                Add("localHost", obj =>
                    ((obj as ChildInvocationHelper)?._connection as IPConnection)?.LocalEndpoint?.Address);

                Add("localPort", obj =>
                    ((obj as ChildInvocationHelper)?._connection as IPConnection)?.LocalEndpoint?.Port);

                Add("remoteHost", obj =>
                    ((obj as ChildInvocationHelper)?._connection as IPConnection)?.RemoteEndpoint?.Address);

                Add("remotePort", obj =>
                    ((obj as ChildInvocationHelper)?._connection as IPConnection)?.RemoteEndpoint?.Port);

                Add("mcastHost", obj =>
                    ((obj as ChildInvocationHelper)?._connection as UdpConnection)?.MulticastEndpoint?.Address);

                Add("mcastPort", obj =>
                    ((obj as ChildInvocationHelper)?._connection as UdpConnection)?.MulticastEndpoint?.Port);

                Add("endpoint", obj => (obj as ChildInvocationHelper)?._connection.Endpoint);
                Add("endpointTransport", obj => (obj as ChildInvocationHelper)?._connection.Endpoint!.Transport);
                Add("endpointIsDatagram", obj => (obj as ChildInvocationHelper)?._connection.Endpoint!.IsDatagram);
                Add("endpointIsAlwaysSecure", obj => (obj as ChildInvocationHelper)?._connection.Endpoint!.IsAlwaysSecure);
                Add("endpointHost", obj => (obj as ChildInvocationHelper)?._connection.Endpoint!.Host);
                Add("endpointPort", obj => (obj as ChildInvocationHelper)?._connection.Endpoint!.Port);
            }
        }
    }
}
