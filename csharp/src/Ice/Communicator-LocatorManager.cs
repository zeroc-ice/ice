//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    public sealed class LocatorInfo
    {
        internal ILocatorPrx Locator { get; }
        private readonly bool _background;
        private readonly Dictionary<string, Task<IReadOnlyList<Endpoint>?>> _adapterRequests =
            new Dictionary<string, Task<IReadOnlyList<Endpoint>?>>();
        private readonly Dictionary<Identity, Task<Reference?>> _objectRequests =
            new Dictionary<Identity, Task<Reference?>>();
        private readonly Lazy<ILocatorRegistryPrx?> _locatorRegistry;
        private readonly object _mutex = new object();
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

        internal void ClearCache(Reference rf)
        {
            Debug.Assert(rf.IsIndirect);
            if (!rf.IsWellKnown)
            {
                IReadOnlyList<Endpoint>? endpoints = _table.RemoveAdapterEndpoints(rf.AdapterId);
                if (endpoints != null && rf.Communicator.TraceLevels.Location >= 2)
                {
                    Trace("removed endpoints for adapter from locator cache", rf, endpoints);
                }
            }
            else
            {
                Reference? r = _table.RemoveObjectReference(rf.Identity);
                if (r != null)
                {
                    if (!r.IsIndirect)
                    {
                        if (rf.Communicator.TraceLevels.Location >= 2)
                        {
                            Trace("removed endpoints for well-known object from locator cache", rf, r.Endpoints);
                        }
                    }
                    else if (!r.IsWellKnown)
                    {
                        if (rf.Communicator.TraceLevels.Location >= 2)
                        {
                            Trace("removed adapter for well-known object from locator cache", rf, r);
                        }
                        ClearCache(r);
                    }
                }
            }
        }

        internal async ValueTask<(IReadOnlyList<Endpoint>, bool)> GetEndpointsAsync(Reference reference, TimeSpan ttl)
        {
            Debug.Assert(reference.IsIndirect);
            IReadOnlyList<Endpoint>? endpoints = null;
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
                        _ = GetObjectReferenceAsync(reference);
                    }
                    else
                    {
                        resolvedReference = await GetObjectReferenceAsync(reference).ConfigureAwait(false);
                    }
                }

                // If the resolved reference encoding doesn't match, we can't use it. Otherwise, we check if it's
                // an direct reference with endpoints or an indirect reference (in which case we need to resolve
                // its endpoints).
                if (resolvedReference != null && resolvedReference.Encoding == reference.Encoding)
                {
                    if (resolvedReference.IsIndirect)
                    {
                        Debug.Assert(!resolvedReference.IsWellKnown);

                        // Get the endpoints for the adapter from the resolved reference.
                        bool adapterCached;
                        (endpoints, adapterCached) = _table.GetAdapterEndpoints(resolvedReference.AdapterId, ttl);
                        if (!adapterCached)
                        {
                            if (_background && endpoints != null)
                            {
                                // Endpoints are returned from the cache but TTL was reached, if backgrounds updates
                                // are configured, we obtain new endpoints but continue using the stale endpoints to
                                // not block the caller.
                                _ = GetAdapterEndpointsAsync(resolvedReference);
                            }
                            else
                            {
                                bool adapterNotFound = false;
                                try
                                {
                                    endpoints = await GetAdapterEndpointsAsync(resolvedReference).ConfigureAwait(false);
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
                                    if (endpoints == null || adapterNotFound)
                                    {
                                        _table.RemoveObjectReference(reference.Identity);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        endpoints = resolvedReference.Endpoints;
                    }
                }
            }
            else
            {
                (endpoints, cached) = _table.GetAdapterEndpoints(reference.AdapterId, ttl);
                if (!cached)
                {
                    if (_background && endpoints != null)
                    {
                        // Endpoints are returned from the cache but TTL was reached, if backgrounds updates
                        // are configured, we obtain new endpoints but continue using the stale endpoints to
                        // not block the caller.
                        _ = GetAdapterEndpointsAsync(reference);
                    }
                    else
                    {
                        endpoints = await GetAdapterEndpointsAsync(reference).ConfigureAwait(false);
                    }
                }
            }

            if (reference.Communicator.TraceLevels.Location >= 1)
            {
                if (endpoints != null && endpoints.Count > 0)
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
                                  reference, endpoints);
                        }
                        else
                        {
                            Trace("retrieved endpoints for adapter from locator, adding to locator cache",
                                  reference, endpoints);
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
            return (endpoints ?? Array.Empty<Endpoint>(), cached);
        }

        private async Task<IReadOnlyList<Endpoint>?> GetAdapterEndpointsAsync(Reference reference)
        {
            if (reference.Communicator.TraceLevels.Location > 0)
            {
                reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocationCat,
                    $"searching for adapter by id\nadapter = {reference.AdapterId}");
            }

            Task<IReadOnlyList<Endpoint>?>? task;
            lock (_mutex)
            {
                if (!_adapterRequests.TryGetValue(reference.AdapterId, out task))
                {
                    // If there's no locator request in progress for this adapter, we invoke one and cache it to prevent
                    // making too many requests on the locator. It's removed once the locator response is received.
                    task = PerformGetAdapterEndpointsAsync(reference);
                    if (!task.IsCompleted)
                    {
                        _adapterRequests.Add(reference.AdapterId, task);
                    }
                }
            }

            try
            {
                // Wait and return the locator response.
                return await task.ConfigureAwait(false);
            }
            catch (AdapterNotFoundException)
            {
                if (reference.Communicator.TraceLevels.Location > 0)
                {
                    reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocationCat,
                        $"adapter not found\nadapter = {reference.AdapterId}");
                }
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

            async Task<IReadOnlyList<Endpoint>?> PerformGetAdapterEndpointsAsync(Reference reference)
            {
                try
                {
                    IObjectPrx? proxy = await Locator.FindAdapterByIdAsync(reference.AdapterId).ConfigureAwait(false);
                    if (proxy != null && !proxy.IceReference.IsIndirect)
                    {
                        // Cache the adapter endpoints.
                        _table.AddAdapterEndpoints(reference.AdapterId, proxy.IceReference.Endpoints);
                        return proxy.IceReference.Endpoints;
                    }
                    else
                    {
                        return null;
                    }
                }
                catch (AdapterNotFoundException)
                {
                    _table.RemoveAdapterEndpoints(reference.AdapterId);
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

        private async Task<Reference?> GetObjectReferenceAsync(Reference reference)
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
                        _objectRequests.Add(reference.Identity, task);
                    }
                }
            }

            try
            {
                // Wait and return the locator response.
                return await task.ConfigureAwait(false);
            }
            catch (AdapterNotFoundException)
            {
                if (reference.Communicator.TraceLevels.Location > 0)
                {
                    reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocationCat,
                        "object not found\n" +
                        $"object = {reference.Identity.ToString(reference.Communicator.ToStringMode)}");
                }
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

            async Task<Reference?> PerformGetObjectProxyAsync(Reference reference)
            {
                try
                {
                    IObjectPrx? proxy = await Locator.FindObjectByIdAsync(reference.Identity).ConfigureAwait(false);
                    if (proxy != null && !proxy.IceReference.IsWellKnown)
                    {
                        // Cache the object reference.
                        _table.AddObjectReference(reference.Identity, proxy.IceReference);
                        return proxy.IceReference;
                    }
                    else
                    {
                        return null;
                    }
                }
                catch (AdapterNotFoundException)
                {
                    _table.RemoveObjectReference(reference.Identity);
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

    public sealed partial class Communicator
    {
        private readonly bool _backgroundLocatorCacheUpdates;
        private readonly ConcurrentDictionary<ILocatorPrx, LocatorInfo> _locatorInfoMap =
            new ConcurrentDictionary<ILocatorPrx, LocatorInfo>();
        private readonly ConcurrentDictionary<(Identity, Encoding), LocatorTable> _locatorTableMap =
            new ConcurrentDictionary<(Identity, Encoding), LocatorTable>();

        // Returns locator info for a given locator. Automatically creates the locator info if it doesn't exist yet.
        internal LocatorInfo? GetLocatorInfo(ILocatorPrx? locator, Encoding encoding)
        {
            if (locator == null)
            {
                return null;
            }

            if (locator.Locator != null || locator.Encoding != encoding)
            {
                // The locator can't be located.
                locator = locator.Clone(clearLocator: true, encoding: encoding);
            }

            return _locatorInfoMap.GetOrAdd(locator, locatorKey => {
                // Rely on locator identity and encoding for the adapter table. We want to have only one table per
                // locator and encoding (not one per locator proxy).
                LocatorTable table =
                    _locatorTableMap.GetOrAdd((locatorKey.Identity, locatorKey.Encoding), key => new LocatorTable());
                return new LocatorInfo(locatorKey, table, _backgroundLocatorCacheUpdates);
            });
        }
    }

    internal sealed class LocatorTable
    {
        private readonly ConcurrentDictionary<string, (TimeSpan Time, IReadOnlyList<Endpoint> Endpoints)>
            _adapterEndpointsTable =
                new ConcurrentDictionary<string, (TimeSpan Time, IReadOnlyList<Endpoint> Endpoints)>();

        private readonly ConcurrentDictionary<Identity, (TimeSpan Time, Reference Reference)>
            _objectReferenceTable =
                new ConcurrentDictionary<Identity, (TimeSpan Time, Reference Reference)>();

        internal void AddAdapterEndpoints(string adapter, IReadOnlyList<Endpoint> endpoints) =>
            _adapterEndpointsTable[adapter] = (Time.CurrentMonotonicTime(), endpoints);

        internal void AddObjectReference(Identity id, Reference reference) =>
            _objectReferenceTable[id] = (Time.CurrentMonotonicTime(), reference);

        internal (IReadOnlyList<Endpoint>?, bool) GetAdapterEndpoints(string adapter, TimeSpan ttl)
        {
            if (ttl == TimeSpan.Zero) // Locator cache disabled.
            {
                return (null, false);
            }

            if (_adapterEndpointsTable.TryGetValue(adapter,
                out (TimeSpan Time, IReadOnlyList<Endpoint> Endpoints) entry))
            {
                return (entry.Endpoints, CheckTTL(entry.Time, ttl));
            }
            else
            {
                return (null, false);
            }
        }

        internal (Reference?, bool) GetObjectReference(Identity id, TimeSpan ttl)
        {
            if (ttl == TimeSpan.Zero) // Locator cache disabled.
            {
                return (null, false);
            }

            if (_objectReferenceTable.TryGetValue(id, out (TimeSpan Time, Reference Reference) entry))
            {
                return (entry.Reference, CheckTTL(entry.Time, ttl));
            }
            else
            {
                return (null, false);
            }
        }

        internal IReadOnlyList<Endpoint>? RemoveAdapterEndpoints(string adapter) =>
            _adapterEndpointsTable.TryRemove(adapter,
                out (TimeSpan Time, IReadOnlyList<Endpoint> Endpoints) entry) ? entry.Endpoints : null;

        internal Reference? RemoveObjectReference(Identity id) =>
            _objectReferenceTable.TryRemove(id,
                out (TimeSpan Time, Reference Reference) entry) ? entry.Reference : null;

        // Returns true if the given timestamp is still valid when compared to the given time-to-live
        private static bool CheckTTL(TimeSpan timestamp, TimeSpan ttl) =>
            ttl == Timeout.InfiniteTimeSpan || (Time.CurrentMonotonicTime() - timestamp) <= ttl;
    }
}
