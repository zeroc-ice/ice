//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The locator info class caches information specific to a given locator proxy. The communicator holds a
    /// locator info instance per locator proxy set either with Ice.Default.Locator or the proxy's Locator property. It
    /// caches the locator registry proxy and keeps track of requests to the locator to prevent multiple concurrent
    /// locator requests for the same adapter or well-known object. The locator info also holds a reference on a locator
    /// table that caches endpoints and resolved references for adapters or well-known objects.
    /// TODO: refactor to support new Locator API (for new protocol/encoding). We could consider merging the locator
    /// table and locator info classes as well.</summary>
    internal sealed class LocatorInfo
    {
        internal ILocatorPrx Locator { get; }
        private readonly bool _background;
        private readonly Dictionary<string, Task<IReadOnlyList<Endpoint>>> _adapterRequests =
            new Dictionary<string, Task<IReadOnlyList<Endpoint>>>();
        private readonly Lazy<ILocatorRegistryPrx?> _locatorRegistry;
        private readonly object _mutex = new object();
        private readonly Dictionary<Identity, Task<Reference?>> _objectRequests =
            new Dictionary<Identity, Task<Reference?>>();
        private readonly LocatorTable _table;

        internal LocatorInfo(ILocatorPrx locator, LocatorTable table, bool background)
        {
            Locator = locator;
            _table = table;
            _background = background;

            // The locator registry can't be located and we use ordered endpoint selection in case the locator
            // returned a proxy with some endpoints which are preferred to be tried first.
            _locatorRegistry = new Lazy<ILocatorRegistryPrx?>(
                () => locator.GetRegistry()?.Clone(clearLocator: true,
                                                   endpointSelection: EndpointSelectionType.Ordered));
        }

        internal void ClearCache(Reference reference)
        {
            Debug.Assert(reference.IsIndirect);
            if (!reference.IsWellKnown)
            {
                IReadOnlyList<Endpoint>? endpoints = _table.RemoveAdapterEndpoints(reference.AdapterId);
                if (endpoints != null && reference.Communicator.TraceLevels.Location >= 2)
                {
                    Trace("removed endpoints for adapter from locator cache", reference, endpoints);
                }
            }
            else
            {
                Reference? resolvedReference = _table.RemoveObjectReference(reference.Identity);
                if (resolvedReference != null)
                {
                    if (!resolvedReference.IsIndirect)
                    {
                        if (reference.Communicator.TraceLevels.Location >= 2)
                        {
                            Trace("removed endpoints for well-known object from locator cache",
                                  reference,
                                  resolvedReference.Endpoints);
                        }
                    }
                    else if (!resolvedReference.IsWellKnown)
                    {
                        if (reference.Communicator.TraceLevels.Location >= 2)
                        {
                            Trace("removed adapter for well-known object from locator cache",
                                  reference,
                                  resolvedReference);
                        }
                        ClearCache(resolvedReference);
                    }
                }
            }
        }

        internal async ValueTask<(IReadOnlyList<Endpoint>, bool)> GetEndpointsAsync(
            Reference reference,
            TimeSpan ttl,
            CancellationToken cancel)
        {
            Debug.Assert(reference.IsIndirect);
            IReadOnlyList<Endpoint> endpoints;
            bool cached;
            if (reference.IsWellKnown)
            {
                // First, we resolve the well-known reference. The resolved reference either embeds the endpoints
                // or an adapter ID.
                Reference? resolvedReference;
                (resolvedReference, cached) = _table.GetObjectReference(reference.Identity, ttl);
                if (!cached)
                {
                    if (_background && resolvedReference != null)
                    {
                        // Reference is returned from the cache but TTL was reached, if backgrounds updates
                        // are configured, we obtain a new reference to refresh the cache but use the stale
                        // reference to not block the caller.
                        _ = GetObjectReferenceAsync(reference, cancel: default);
                    }
                    else
                    {
                        resolvedReference = await GetObjectReferenceAsync(reference, cancel).ConfigureAwait(false);
                    }
                }

                if (resolvedReference == null || resolvedReference.Encoding != reference.Encoding)
                {
                    // If the resolved reference is null or the encoding doesn't match, we can't use it.
                    endpoints = ImmutableArray<Endpoint>.Empty;
                }
                else if (!resolvedReference.IsIndirect)
                {
                    // If it's a direct reference, just get its endpoints.
                    endpoints = resolvedReference.Endpoints;
                }
                else
                {
                    // Otherwise, it's an indirect reference (but can't be a well-known reference because
                    // GetObjectReferenceAsync doesn't return well-known references). We need to resolve its endpoints.
                    Debug.Assert(!resolvedReference.IsWellKnown);

                    // Get the endpoints for the adapter from the resolved reference.
                    bool adapterCached;
                    (endpoints, adapterCached) = _table.GetAdapterEndpoints(resolvedReference.AdapterId, ttl);
                    if (!adapterCached)
                    {
                        if (_background && endpoints.Count > 0)
                        {
                            // Endpoints are returned from the cache but TTL was reached, if backgrounds updates
                            // are configured, we obtain new endpoints but continue using the stale endpoints to
                            // not block the caller.
                            _ = GetAdapterEndpointsAsync(resolvedReference, cancel: default);
                        }
                        else
                        {
                            bool adapterNotFound = false;
                            try
                            {
                                endpoints =
                                    await GetAdapterEndpointsAsync(resolvedReference, cancel).ConfigureAwait(false);
                            }
                            catch (AdapterNotFoundException)
                            {
                                adapterNotFound = true;
                                throw;
                            }
                            finally
                            {
                                // If we can't resolve the endpoints, we clear the resolved object reference from
                                // the cache.
                                if (endpoints.Count == 0 || adapterNotFound)
                                {
                                    _table.RemoveObjectReference(reference.Identity);
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                (endpoints, cached) = _table.GetAdapterEndpoints(reference.AdapterId, ttl);
                if (!cached)
                {
                    if (_background && endpoints.Count > 0)
                    {
                        // Endpoints are returned from the cache but TTL was reached, if backgrounds updates
                        // are configured, we obtain new endpoints but continue using the stale endpoints to
                        // not block the caller.
                        _ = GetAdapterEndpointsAsync(reference, cancel: default);
                    }
                    else
                    {
                        endpoints = await GetAdapterEndpointsAsync(reference, cancel).ConfigureAwait(false);
                    }
                }
            }

            if (reference.Communicator.TraceLevels.Location >= 1)
            {
                if (endpoints.Count > 0)
                {
                    if (cached)
                    {
                        if (reference.IsWellKnown)
                        {
                            Trace("found endpoints for well-known proxy in locator cache", reference, endpoints);
                        }
                        else
                        {
                            Trace("found endpoints for adapter in locator cache", reference, endpoints);
                        }
                    }
                    else
                    {
                        if (reference.IsWellKnown)
                        {
                            Trace("retrieved endpoints for well-known proxy from locator, adding to locator cache",
                                  reference,
                                  endpoints);
                        }
                        else
                        {
                            Trace("retrieved endpoints for adapter from locator, adding to locator cache",
                                  reference,
                                  endpoints);
                        }
                    }
                }
                else
                {
                    Communicator communicator = reference.Communicator;
                    var s = new System.Text.StringBuilder();
                    s.Append("no endpoints configured for ");
                    if (reference.AdapterId.Length > 0)
                    {
                        s.Append("adapter\n");
                        s.Append("adapter = " + reference.AdapterId);
                    }
                    else
                    {
                        s.Append("well-known object\n");
                        s.Append("well-known proxy = " + reference.ToString());
                    }
                    communicator.Logger.Trace(communicator.TraceLevels.LocationCat, s.ToString());
                }
            }
            return (endpoints, cached);
        }

        private async Task<IReadOnlyList<Endpoint>> GetAdapterEndpointsAsync(
            Reference reference,
            CancellationToken cancel)
        {
            if (reference.Communicator.TraceLevels.Location > 0)
            {
                reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocationCat,
                    $"searching for adapter by id\nadapter = {reference.AdapterId}");
            }

            Task<IReadOnlyList<Endpoint>>? task;
            lock (_mutex)
            {
                if (!_adapterRequests.TryGetValue(reference.AdapterId, out task))
                {
                    // If there's no locator request in progress for this adapter, we invoke one and cache it to prevent
                    // making too many requests on the locator. It's removed once the locator response is received.
                    task = PerformGetAdapterEndpointsAsync(reference);
                    if (!task.IsCompleted)
                    {
                        // If PerformGetAdapterEndpointsAsync completed, don't add the task (it would leak since
                        // PerformGetAdapterEndpointsAsync is responsible for removing it).
                        _adapterRequests.Add(reference.AdapterId, task);
                    }
                }
            }

            return await task.WaitAsync(cancel).ConfigureAwait(false);

            async Task<IReadOnlyList<Endpoint>> PerformGetAdapterEndpointsAsync(Reference reference)
            {
                try
                {
                    // TODO: Fix FindAdapterById to return non-null proxy
                    IObjectPrx? proxy = await Locator.FindAdapterByIdAsync(reference.AdapterId).ConfigureAwait(false);
                    if (proxy != null && !proxy.IceReference.IsIndirect)
                    {
                        // Cache the adapter endpoints.
                        _table.SetAdapterEndpoints(reference.AdapterId, proxy.IceReference.Endpoints);
                        return proxy.IceReference.Endpoints;
                    }
                    else
                    {
                        return ImmutableArray<Endpoint>.Empty;
                    }
                }
                catch (AdapterNotFoundException)
                {
                    if (reference.Communicator.TraceLevels.Location > 0)
                    {
                        reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocationCat,
                            $"adapter not found\nadapter = {reference.AdapterId}");
                    }
                    _table.RemoveAdapterEndpoints(reference.AdapterId);
                    throw;
                }
                catch (Exception exception)
                {
                    if (reference.Communicator.TraceLevels.Location > 0)
                    {
                        reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocationCat,
                            "could not contact the locator to retrieve endpoints\n" +
                            $"adapter = {reference.AdapterId}\nreason = {exception}");
                    }
                    throw;
                }
                finally
                {
                    lock (_mutex)
                    {
                        _adapterRequests.Remove(reference.AdapterId);
                    }
                }
            }
        }

        internal ILocatorRegistryPrx? GetLocatorRegistry() => _locatorRegistry.Value;

        private async Task<Reference?> GetObjectReferenceAsync(Reference reference, CancellationToken cancel)
        {
            if (reference.Communicator.TraceLevels.Location > 0)
            {
                reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocationCat,
                    $"searching for well-known object\nwell-known proxy = {reference}");
            }

            Task<Reference?>? task;
            lock (_mutex)
            {
                if (!_objectRequests.TryGetValue(reference.Identity, out task))
                {
                    // If there's no locator request in progress for this object, we make one and cache it to prevent
                    // making too many requests on the locator. It's removed once the locator response is received.
                    task = PerformGetObjectProxyAsync(reference);
                    if (!task.IsCompleted)
                    {
                        // If PerformGetObjectProxyAsync completed, don't add the task (it would leak since
                        // PerformGetObjectProxyAsync is responsible for removing it).
                        _objectRequests.Add(reference.Identity, task);
                    }
                }
            }

            return await task.WaitAsync(cancel).ConfigureAwait(false);

            async Task<Reference?> PerformGetObjectProxyAsync(Reference reference)
            {
                try
                {
                    // TODO: Fix FindObjectById to return non-null proxy
                    IObjectPrx? proxy = await Locator.FindObjectByIdAsync(reference.Identity).ConfigureAwait(false);
                    if (proxy != null && !proxy.IceReference.IsWellKnown)
                    {
                        // Cache the object reference.
                        _table.SetObjectReference(reference.Identity, proxy.IceReference);
                        return proxy.IceReference;
                    }
                    else
                    {
                        return null;
                    }
                }
                catch (ObjectNotFoundException)
                {
                    if (reference.Communicator.TraceLevels.Location > 0)
                    {
                        reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocationCat,
                            "object not found\n" +
                            $"object = {reference.Identity.ToString(reference.Communicator.ToStringMode)}");
                    }
                    _table.RemoveObjectReference(reference.Identity);
                    throw;
                }
                catch (Exception exception)
                {
                    if (reference.Communicator.TraceLevels.Location > 0)
                    {
                        reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocationCat,
                            "could not contact the locator to retrieve endpoints\n" +
                            $"well-known proxy = {reference}\nreason = {exception}");
                    }
                    throw;
                }
                finally
                {
                    lock (_mutex)
                    {
                        _objectRequests.Remove(reference.Identity);
                    }
                }
            }
        }

        private static void Trace(string msg, Reference r, IReadOnlyList<Endpoint> endpoints)
        {
            var s = new System.Text.StringBuilder();
            s.Append(msg + "\n");
            if (r.AdapterId.Length > 0)
            {
                s.Append("adapter = " + r.AdapterId + "\n");
            }
            else
            {
                s.Append("well-known proxy = " + r.ToString() + "\n");
            }
            s.Append("endpoints = ");
            s.Append(string.Join(":", endpoints));
            r.Communicator.Logger.Trace(r.Communicator.TraceLevels.LocationCat, s.ToString());
        }

        private static void Trace(string msg, Reference r, Reference resolved)
        {
            Debug.Assert(r.IsWellKnown);
            var s = new System.Text.StringBuilder();
            s.Append(msg);
            s.Append("\n");
            s.Append("well-known proxy = ");
            s.Append(r.ToString());
            s.Append("\n");
            s.Append("adapter = ");
            s.Append(resolved.AdapterId);
            r.Communicator.Logger.Trace(r.Communicator.TraceLevels.LocationCat, s.ToString());
        }
    }

    /// <summary>The LocatorTable class caches endpoints and references for adapters and well-known objects resolved
    /// through a given locator. The communicator maintains a dictionary of locator tables for each locator identity,
    /// and encoding pair. A locator table can be used by multiple locator info instances.</summary>
    internal sealed class LocatorTable
    {
        private readonly ConcurrentDictionary<string, (TimeSpan InsertionTime, IReadOnlyList<Endpoint> Endpoints)>
            _adapterEndpointsTable =
                new ConcurrentDictionary<string, (TimeSpan InsertionTime, IReadOnlyList<Endpoint> Endpoints)>();

        private readonly ConcurrentDictionary<Identity, (TimeSpan InsertionTime, Reference Reference)>
            _objectReferenceTable =
                new ConcurrentDictionary<Identity, (TimeSpan InsertionTime, Reference Reference)>();

        internal void SetAdapterEndpoints(string adapter, IReadOnlyList<Endpoint> endpoints) =>
            _adapterEndpointsTable[adapter] = (Time.CurrentMonotonicTime(), endpoints);

        internal void SetObjectReference(Identity id, Reference reference) =>
            _objectReferenceTable[id] = (Time.CurrentMonotonicTime(), reference);

        internal (IReadOnlyList<Endpoint> Endpoints, bool Cached) GetAdapterEndpoints(string adapter, TimeSpan ttl)
        {
            if (ttl == TimeSpan.Zero) // Locator cache disabled.
            {
                return (ImmutableArray<Endpoint>.Empty, false);
            }

            if (_adapterEndpointsTable.TryGetValue(adapter,
                out (TimeSpan InsertionTime, IReadOnlyList<Endpoint> Endpoints) entry))
            {
                return (entry.Endpoints, CheckTTL(entry.InsertionTime, ttl));
            }
            else
            {
                return (ImmutableArray<Endpoint>.Empty, false);
            }
        }

        internal (Reference? Reference, bool Cached) GetObjectReference(Identity id, TimeSpan ttl)
        {
            if (ttl == TimeSpan.Zero) // Locator cache disabled.
            {
                return (null, false);
            }

            if (_objectReferenceTable.TryGetValue(id, out (TimeSpan InsertionTime, Reference Reference) entry))
            {
                return (entry.Reference, CheckTTL(entry.InsertionTime, ttl));
            }
            else
            {
                return (null, false);
            }
        }

        internal IReadOnlyList<Endpoint>? RemoveAdapterEndpoints(string adapter) =>
            _adapterEndpointsTable.TryRemove(adapter,
                out (TimeSpan InsertionTime, IReadOnlyList<Endpoint> Endpoints) entry) ? entry.Endpoints : null;

        internal Reference? RemoveObjectReference(Identity id) =>
            _objectReferenceTable.TryRemove(id,
                out (TimeSpan InsertionTime, Reference Reference) entry) ? entry.Reference : null;

        // Returns true if the time-to-live has been reached
        private static bool CheckTTL(TimeSpan insertionTime, TimeSpan ttl) =>
            ttl == Timeout.InfiniteTimeSpan || (Time.CurrentMonotonicTime() - insertionTime) <= ttl;
    }
}
