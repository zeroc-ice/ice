//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    public sealed class LocatorInfo
    {
        internal ILocatorPrx Locator { get; }
        private readonly Lazy<ILocatorRegistryPrx?> _locatorRegistry;
        private readonly LocatorTable _table;
        private readonly bool _background;
        private readonly Dictionary<string, Task<IReadOnlyList<Endpoint>?>> _adapterRequests =
            new Dictionary<string, Task<IReadOnlyList<Endpoint>?>>();
        private readonly Dictionary<Identity, Task<Reference?>> _objectRequests =
            new Dictionary<Identity, Task<Reference?>>();

        public override bool Equals(object? obj) =>
             ReferenceEquals(this, obj) || (obj is LocatorInfo rhs && Locator.Equals(rhs.Locator));

        public override int GetHashCode() => Locator.GetHashCode();

        internal LocatorInfo(ILocatorPrx locator, LocatorTable table, bool background)
        {
            Locator = locator;
            _table = table;
            _background = background;

            // The locator registry can't be located. We use ordered endpoint selection in case the locator
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

        internal async ValueTask<(IReadOnlyList<Endpoint>, bool)> GetEndpointsAsync(Reference reference, int ttl)
        {
            Debug.Assert(reference.IsIndirect);
            IReadOnlyList<Endpoint>? endpoints = null;
            bool cached;
            if (!reference.IsWellKnown)
            {
                (endpoints, cached) = _table.GetAdapterEndpoints(reference.AdapterId, ttl);
                if (!cached)
                {
                    if (_background && endpoints != null)
                    {
                        _ = GetAdapterEndpointsAsync(reference);
                    }
                    else
                    {
                        endpoints = await GetAdapterEndpointsAsync(reference).ConfigureAwait(false);
                    }
                }
            }
            else
            {
                Reference? referenceForWellKnown;
                (referenceForWellKnown, cached) = _table.GetObjectReference(reference.Identity, ttl);
                if (!cached)
                {
                    if (_background && referenceForWellKnown != null)
                    {
                        _ = GetObjectReferenceAsync(reference);
                    }
                    else
                    {
                        referenceForWellKnown = await GetObjectReferenceAsync(reference).ConfigureAwait(false);
                    }
                }

                if (referenceForWellKnown != null)
                {
                    Debug.Assert(referenceForWellKnown.Encoding == reference.Encoding);
                    if (referenceForWellKnown.IsIndirect)
                    {
                        Debug.Assert(!referenceForWellKnown.IsWellKnown);

                        bool adapterCached;
                        (endpoints, adapterCached) = _table.GetAdapterEndpoints(referenceForWellKnown.AdapterId, ttl);
                        if (!adapterCached)
                        {
                            if (_background && endpoints != null)
                            {
                                _ = GetAdapterEndpointsAsync(referenceForWellKnown);
                            }
                            else
                            {
                                // Get the endpoints for the adapter ID from the resolved well-known reference. If no
                                // endpoints are returned or the adapter is not found, we clear the resolved well-known
                                // reference from the cache.
                                try
                                {
                                    endpoints =
                                        await GetAdapterEndpointsAsync(referenceForWellKnown).ConfigureAwait(false);
                                    if (endpoints == null)
                                    {
                                        _table.RemoveObjectReference(referenceForWellKnown.Identity);
                                    }
                                }
                                catch (AdapterNotFoundException)
                                {
                                    _table.RemoveObjectReference(referenceForWellKnown.Identity);
                                    throw;
                                }
                            }
                        }
                    }
                    else
                    {
                        endpoints = referenceForWellKnown.Endpoints;
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
            lock (_adapterRequests)
            {
                if (!_adapterRequests.TryGetValue(reference.AdapterId, out task))
                {
                    // If there's no locator request in progress for this adapter, we make one and cache it to prevent
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
                    lock (_adapterRequests)
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
            lock (_objectRequests)
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
                    if (proxy != null && !proxy.IceReference.IsWellKnown &&
                        proxy.IceReference.Encoding == reference.Encoding)
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
                    lock (_objectRequests)
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
        private readonly Dictionary<ILocatorPrx, LocatorInfo> _locatorInfoMap =
            new Dictionary<ILocatorPrx, LocatorInfo>();
        private readonly Dictionary<LocatorKey, LocatorTable> _locatorTableMap =
            new Dictionary<LocatorKey, LocatorTable>();
        private readonly bool _backgroundLocatorCacheUpdates;

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

            // TODO: reap unused locator info objects?
            lock (_locatorInfoMap)
            {
                if (!_locatorInfoMap.TryGetValue(locator, out LocatorInfo? info))
                {
                    // Rely on locator identity for the adapter table. We want to have only one table per locator
                    // (not one per locator proxy).
                    var key = new LocatorKey(locator);
                    if (!_locatorTableMap.TryGetValue(key, out LocatorTable? table))
                    {
                        table = new LocatorTable();
                        _locatorTableMap[key] = table;
                    }

                    info = new LocatorInfo(locator, table, _backgroundLocatorCacheUpdates);
                    _locatorInfoMap[locator] = info;
                }

                return info;
            }
        }

        private readonly struct LocatorKey : IEquatable<LocatorKey>
        {
            private readonly Identity _id;
            private readonly Encoding _encoding;

            public LocatorKey(ILocatorPrx prx)
            {
                _id = prx.Identity;
                _encoding = prx.Encoding;
            }

            public bool Equals(LocatorKey other) => _id.Equals(other._id) && _encoding.Equals(other._encoding);

            public override bool Equals(object? obj) => (obj is LocatorKey other) && Equals(other);

            public override int GetHashCode() => HashCode.Combine(_id, _encoding);
        }
    }

    internal sealed class LocatorTable
    {
        private readonly Dictionary<string, (long Time, IReadOnlyList<Endpoint> Endpoints)> _adapterEndpointsTable =
            new Dictionary<string, (long Time, IReadOnlyList<Endpoint> Endpoints)>();
        private readonly Dictionary<Identity, (long Time, Reference Reference)> _objectTable =
            new Dictionary<Identity, (long Time, Reference Reference)>();

        internal void AddAdapterEndpoints(string adapter, IReadOnlyList<Endpoint> endpoints)
        {
            lock (_adapterEndpointsTable)
            {
                _adapterEndpointsTable[adapter] = (Time.CurrentMonotonicTimeMillis(), endpoints);
            }
        }

        internal void AddObjectReference(Identity id, Reference reference)
        {
            lock (_objectTable)
            {
                _objectTable[id] = (Time.CurrentMonotonicTimeMillis(), reference);
            }
        }

        internal (IReadOnlyList<Endpoint>?, bool) GetAdapterEndpoints(string adapter, int ttl)
        {
            if (ttl == 0) // Locator cache disabled.
            {
                return (null, false);
            }

            lock (_adapterEndpointsTable)
            {
                if (_adapterEndpointsTable.TryGetValue(adapter,
                    out (long Time, IReadOnlyList<Endpoint> Endpoints) entry))
                {
                    return (entry.Endpoints, CheckTTL(entry.Time, ttl));
                }
                return (null, false);
            }
        }

        internal (Reference?, bool) GetObjectReference(Identity id, int ttl)
        {
            if (ttl == 0) // Locator cache disabled.
            {
                return (null, false);
            }

            lock (_objectTable)
            {
                if (_objectTable.TryGetValue(id, out (long Time, Reference Reference) entry))
                {
                    return (entry.Reference, CheckTTL(entry.Time, ttl));
                }
                return (null, false);
            }
        }

        internal IReadOnlyList<Endpoint>? RemoveAdapterEndpoints(string adapter)
        {
            lock (_adapterEndpointsTable)
            {
                if (_adapterEndpointsTable.TryGetValue(adapter,
                    out (long Time, IReadOnlyList<Endpoint> Endpoints) entry))
                {
                    _adapterEndpointsTable.Remove(adapter);
                    return entry.Endpoints;
                }
                return null;
            }
        }

        internal Reference? RemoveObjectReference(Identity id)
        {
            lock (_objectTable)
            {
                if (_objectTable.TryGetValue(id, out (long Time, Reference Reference) entry))
                {
                    _objectTable.Remove(id);
                    return entry.Reference;
                }
                return null;
            }
        }

        private static bool CheckTTL(long time, int ttl)
        {
            Debug.Assert(ttl != 0);
            if (ttl < 0) // TTL = infinite
            {
                return true;
            }
            else
            {
                return Time.CurrentMonotonicTimeMillis() - time <= ((long)ttl * 1000);
            }
        }

        private readonly struct TableEntry<T>
        {
            public TableEntry(long time, T value)
            {
                Time = time;
                Value = value;
            }

            public readonly long Time;
            public readonly T Value;
        }
    }
}
