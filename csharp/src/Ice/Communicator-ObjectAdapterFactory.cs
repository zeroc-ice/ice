// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    public sealed partial class Communicator
    {
        /// <summary>Returns a task that completes when the communicator's shutdown is complete: see
        /// <see cref="ShutdownAsync"/>. This property can be retrieved before shutdown is initiated. A typical use-case
        /// is to call <c>await communicator.ShutdownComplete;</c> in the Main method of a server to prevent the server
        /// from exiting immediately. Once this task completes, the server can still make remote invocations since a
        /// communicator that is shut down (but not disposed) remains usable for remote invocations.</summary>
        public Task ShutdownComplete => _shutdownCompleteSource.Task;

        /// <summary>Shuts down this communicator's server functionality. This triggers the disposal of all object
        /// adapters. After this method returns, no new requests are processed. However, requests that have been started
        /// before ShutdownAsync was called might still be active until the returned task completes.</summary>
        public Task ShutdownAsync()
        {
            lock (_mutex)
            {
                _shutdownTask ??= new Lazy<Task>(() => PerformShutdownAsync());
            }
            return _shutdownTask.Value;

            async Task PerformShutdownAsync()
            {
                try
                {
                    // _adapters can only be updated when _shutdownTask is null so no need to lock _mutex.
                    await Task.WhenAll(_adapters.Select(adapter => adapter.ShutdownAsync())).ConfigureAwait(false);
                }
                finally
                {
                    // The continuation is executed asynchronously (see _shutdownCompleteSource's construction). This
                    // way, even if the continuation blocks waiting on ShutdownAsync to complete (with incorrect code
                    // using Result or Wait()), ShutdownAsync will complete.
                    _shutdownCompleteSource.TrySetResult(null);
                }
            }
        }

        /// <summary>Creates a new nameless object adapter. Such an object adapter has no configuration and can be
        /// associated with a bidirectional connection.</summary>
        /// <param name="serializeDispatch">Indicates whether or not this object adapter serializes the dispatching of
        /// of requests received over the same connection.</param>
        /// <param name="taskScheduler">The optional task scheduler to use for dispatching requests.</param>
        /// <param name="protocol">The protocol used for this object adapter.</param>
        /// <returns>The new object adapter.</returns>
        public ObjectAdapter CreateObjectAdapter(
            bool serializeDispatch = false,
            TaskScheduler? taskScheduler = null,
            Protocol protocol = Protocol.Ice2)
        {
            lock (_mutex)
            {
                if (IsDisposed)
                {
                    throw new CommunicatorDisposedException();
                }
                if (_shutdownTask != null)
                {
                    throw new InvalidOperationException("ShutdownAsync has been called on this communicator");
                }

                var adapter = new ObjectAdapter(this, serializeDispatch, taskScheduler, protocol);
                _adapters.Add(adapter);
                return adapter;
            }
        }

        /// <summary>Creates a new object adapter. The communicator uses the object adapter's name to lookup its
        /// properties, such as name.Endpoints.</summary>
        /// <param name="name">The object adapter name. Cannot be empty.</param>
        /// <param name="serializeDispatch">Indicates whether or not this object adapter serializes the dispatching of
        /// of requests received over the same connection.</param>
        /// <param name="taskScheduler">The optional task scheduler to use for dispatching requests.</param>
        /// <returns>The new object adapter.</returns>
        public ObjectAdapter CreateObjectAdapter(
            string name,
            bool serializeDispatch = false,
            TaskScheduler? taskScheduler = null) =>
            CreateObjectAdapter(name, serializeDispatch, taskScheduler, router: null);

        /// <summary>Creates a new object adapter with the specified endpoint string. Calling this method is equivalent
        /// to setting the name.Endpoints property and then calling
        /// <see cref="CreateObjectAdapter(string, bool, TaskScheduler?)"/>.</summary>
        /// <param name="name">The object adapter name. Cannot be empty.</param>
        /// <param name="endpoints">The endpoint string for the object adapter.</param>
        /// <param name="serializeDispatch">Indicates whether or not this object adapter serializes the dispatching of
        /// of requests received over the same connection.</param>
        /// <param name="taskScheduler">The optional task scheduler to use for dispatching requests.</param>
        /// <returns>The new object adapter.</returns>
        public ObjectAdapter CreateObjectAdapterWithEndpoints(
            string name,
            string endpoints,
            bool serializeDispatch = false,
            TaskScheduler? taskScheduler = null)
        {
            if (name.Length == 0)
            {
                throw new ArgumentException("the empty string is not a valid object adapter name", nameof(name));
            }

            SetProperty($"{name}.Endpoints", endpoints);
            return CreateObjectAdapter(name, serializeDispatch, taskScheduler);
        }

        /// <summary>Creates a new object adapter with the specified endpoint string. This method generates a UUID for
        /// the object adapter name and then calls
        /// <see cref="CreateObjectAdapterWithEndpoints(string, string, bool, TaskScheduler?)"/>.
        /// </summary>
        /// <param name="endpoints">The endpoint string for the object adapter.</param>
        /// <param name="serializeDispatch">Indicates whether or not this object adapter serializes the dispatching of
        /// of requests received over the same connection.</param>
        /// <param name="taskScheduler">The optional task scheduler to use for dispatching requests.</param>
        /// <returns>The new object adapter.</returns>
        public ObjectAdapter CreateObjectAdapterWithEndpoints(
            string endpoints,
            bool serializeDispatch = false,
            TaskScheduler? taskScheduler = null) =>
            CreateObjectAdapterWithEndpoints(Guid.NewGuid().ToString(),
                                             endpoints,
                                             serializeDispatch,
                                             taskScheduler);

        /// <summary>Creates a new object adapter with a router.</summary>
        /// <param name="name">The object adapter name. Cannot be empty.</param>
        /// <param name="router">The proxy to the router.</param>
        /// <param name="serializeDispatch">Indicates whether or not this object adapter serializes the dispatching of
        /// of requests received over the same connection.</param>
        /// <param name="taskScheduler">The optional task scheduler to use for dispatching requests.</param>
        /// <returns>The new object adapter.</returns>
        public ObjectAdapter CreateObjectAdapterWithRouter(
            string name,
            IRouterPrx router,
            bool serializeDispatch = false,
            TaskScheduler? taskScheduler = null)
        {
            if (name.Length == 0)
            {
                throw new ArgumentException("the empty string is not a valid object adapter name", nameof(name));
            }

            // We set the proxy properties here, although we still use the proxy supplied.
            Dictionary<string, string> properties = router.ToProperty($"{name}.Router");
            foreach (KeyValuePair<string, string> entry in properties)
            {
                SetProperty(entry.Key, entry.Value);
            }

            return CreateObjectAdapter(name, serializeDispatch, taskScheduler, router);
        }

        /// <summary>Creates a new object adapter with a router. This method generates a UUID for the object adapter
        /// name and then calls <see cref="CreateObjectAdapterWithRouter(string, IRouterPrx, bool, TaskScheduler?)"/>.
        /// </summary>
        /// <param name="router">The proxy to the router.</param>
        /// <param name="serializeDispatch">Indicates whether or not this object adapter serializes the dispatching of
        /// of requests received over the same connection.</param>
        /// <param name="taskScheduler">The optional task scheduler to use for dispatching requests.</param>
        /// <returns>The new object adapter.</returns>
        public ObjectAdapter CreateObjectAdapterWithRouter(
            IRouterPrx router,
            bool serializeDispatch = false,
            TaskScheduler? taskScheduler = null) =>
            CreateObjectAdapterWithRouter(Guid.NewGuid().ToString(), router, serializeDispatch, taskScheduler);

        internal Endpoint? GetColocatedEndpoint(Reference reference)
        {
            List<ObjectAdapter> adapters;
            lock (_mutex)
            {
                if (IsDisposed)
                {
                    throw new CommunicatorDisposedException();
                }
                adapters = new List<ObjectAdapter>(_adapters);
            }

            foreach (ObjectAdapter adapter in adapters)
            {
                try
                {
                    if (adapter.IsLocal(reference))
                    {
                        return adapter.GetColocatedEndpoint();
                    }
                }
                catch (ObjectDisposedException)
                {
                    // Ignore.
                }
            }

            return null;
        }

        internal void RemoveObjectAdapter(ObjectAdapter adapter)
        {
            // Called by the object adapter to remove itself once destroyed.
            lock (_mutex)
            {
                if (_shutdownTask == null)
                {
                    _adapters.Remove(adapter);
                    if (adapter.Name.Length > 0)
                    {
                        _adapterNamesInUse.Remove(adapter.Name);
                    }
                }
                // TODO clear outgoing connections adapter?
            }
        }

        private ObjectAdapter CreateObjectAdapter(
            string name,
            bool serializeDispatch,
            TaskScheduler? taskScheduler,
            IRouterPrx? router)
        {
            if (name.Length == 0)
            {
                throw new ArgumentException("the empty string is not a valid object adapter name", nameof(name));
            }

            lock (_mutex)
            {
                if (IsDisposed)
                {
                    throw new CommunicatorDisposedException();
                }
                if (_shutdownTask != null)
                {
                    throw new InvalidOperationException("ShutdownAsync has been called on this communicator");
                }

                if (!_adapterNamesInUse.Add(name))
                {
                    throw new ArgumentException($"an object adapter with name `{name}' was already created",
                                                nameof(name));
                }

                try
                {
                    var adapter = new ObjectAdapter(this, name, serializeDispatch, taskScheduler, router);
                    _adapters.Add(adapter);
                    return adapter;
                }
                catch
                {
                    _adapterNamesInUse.Remove(name);
                    throw;
                }
            }
        }
    }
}
