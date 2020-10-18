// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>The locator info class caches information specific to a given locator proxy. The communicator holds a
    /// locator info instance per locator proxy set either with Ice.Default.Locator or the proxy's Locator property. It
    /// caches the locator registry proxy and keeps track of requests to the locator to prevent multiple concurrent
    /// identical requests.</summary>
    internal sealed class LocatorInfo
    {
        internal ILocatorPrx Locator { get; }

        private static readonly IEqualityComparer<Reference> _locationComparer = new LocationComparer();
        private static readonly IEqualityComparer<Reference> _wellKnownProxyComparer = new WellKnownProxyComparer();

        private readonly bool _background;

        private readonly ConcurrentDictionary<Reference, (TimeSpan InsertionTime, Reference Reference)>
            _locationCache = new (_locationComparer);

        private readonly Dictionary<Reference, Task<Reference?>> _locationRequests = new (_locationComparer);

        private ILocatorRegistryPrx? _locatorRegistry;

        // _mutex protects _locationRequests and _wellKnownProxyRequests
        private readonly object _mutex = new ();

        private readonly ConcurrentDictionary<Reference, (TimeSpan InsertionTime, Reference Reference)>
            _wellKnownProxyCache = new (_wellKnownProxyComparer);

        private readonly Dictionary<Reference, Task<Reference?>> _wellKnownProxyRequests =
            new (_wellKnownProxyComparer);

        internal LocatorInfo(ILocatorPrx locator, bool background)
        {
            Locator = locator;
            _background = background;
        }

        internal void ClearCache(Reference reference)
        {
            Debug.Assert(reference.IsIndirect);

            if (reference.IsWellKnown)
            {
                if (RemoveWellKnownProxy(reference) is Reference resolvedWellKnownProxy)
                {
                    if (resolvedWellKnownProxy.IsIndirect)
                    {
                        // We don't cache bogus well-known resolved references.
                        Debug.Assert(!resolvedWellKnownProxy.IsWellKnown);

                        if (reference.Communicator.TraceLevels.Locator >= 2)
                        {
                            TraceWellKnown("removed well-known proxy without endpoints from locator cache",
                                           reference,
                                           resolvedWellKnownProxy);
                        }
                        ClearCache(resolvedWellKnownProxy); // i.e clears location cache
                    }
                    else
                    {
                        if (reference.Communicator.TraceLevels.Locator >= 2)
                        {
                            TraceDirect("removed well-known proxy with endpoints from locator cache",
                                        reference,
                                        resolvedWellKnownProxy);
                        }
                    }
                }
            }
            else
            {
                if (RemoveLocation(reference) is Reference resolvedLocation &&
                    reference.Communicator.TraceLevels.Locator >= 2)
                {
                    TraceDirect("removed endpoints for location from locator cache", reference, resolvedLocation);
                }
            }
        }

        /// <summary>Resolves an indirect reference using the locator proxy or cache.</summary>
        internal async ValueTask<(Reference? DirectReference, bool Cached)> ResolveIndirectReferenceAsync(
            Reference reference,
            CancellationToken cancel)
        {
            Debug.Assert(reference.IsIndirect);
            Reference? directReference = null;
            bool cached = false;

            Reference? indirectReference = null;

            if (reference.IsWellKnown)
            {
                Reference? resolvedWellKnownProxy;

                // First, we resolve the well-known reference. The resolved reference can be direct or indirect, but
                // cannot be well-known.
                (resolvedWellKnownProxy, cached) = GetResolvedWellKnownProxyFromCache(reference,
                                                                                      reference.LocatorCacheTimeout);
                if (!cached)
                {
                    if (_background && resolvedWellKnownProxy != null)
                    {
                        // Reference is returned from the cache but TTL was reached, if backgrounds updates
                        // are configured, we obtain a new reference to refresh the cache but use the stale
                        // reference to not block the caller.
                        _ = ResolveWellKnownProxyAsync(reference, cancel: default);
                    }
                    else
                    {
                        resolvedWellKnownProxy =
                            await ResolveWellKnownProxyAsync(reference, cancel).ConfigureAwait(false);
                    }
                }

                if (resolvedWellKnownProxy != null)
                {
                    if (resolvedWellKnownProxy.IsIndirect)
                    {
                        indirectReference = resolvedWellKnownProxy;
                    }
                    else
                    {
                        directReference = resolvedWellKnownProxy;
                    }
                }
            }
            else
            {
                indirectReference = reference;
            }

            if (indirectReference != null)
            {
                bool cachedLocation;

                (directReference, cachedLocation) =
                    GetResolvedLocationFromCache(indirectReference, reference.LocatorCacheTimeout);

                if (!cachedLocation)
                {
                    if (_background && directReference != null)
                    {
                        // Endpoints are returned from the cache but TTL was reached, if backgrounds updates
                        // are configured, we obtain new endpoints but continue using the stale endpoints to
                        // not block the caller.
                        _ = ResolveLocationAsync(indirectReference, cancel: default);
                    }
                    else
                    {
                        try
                        {
                            directReference =
                                await ResolveLocationAsync(indirectReference, cancel).ConfigureAwait(false);
                        }
                        finally
                        {
                            // If we can't resolve the location we clear the resolved well known proxy from the cache.
                            if (directReference == null && reference.IsWellKnown)
                            {
                                RemoveWellKnownProxy(reference);
                            }
                        }
                    }
                }

                if (!reference.IsWellKnown)
                {
                    cached = cachedLocation;
                }
            }

            if (reference.Communicator.TraceLevels.Locator >= 1)
            {
                if (directReference != null)
                {
                    string kind = reference.IsWellKnown ? "well-known proxy" : "location";
                    TraceDirect(
                            cached ? $"found entry for {kind} in locator cache" :
                                $"resolved {kind} using locator, adding to locator cache",
                            reference,
                            directReference);
                }
                else
                {
                    Communicator communicator = reference.Communicator;
                    var sb = new System.Text.StringBuilder();
                    sb.Append("could not find endpoint(s) for ");
                    if (reference.Location.Count > 0)
                    {
                        sb.Append("location ");
                        sb.Append(reference.LocationAsString);
                    }
                    else
                    {
                        sb.Append("well-known proxy ");
                        sb.Append(reference);
                    }
                    communicator.Logger.Trace(communicator.TraceLevels.LocatorCategory, sb.ToString());
                }
            }

            return (directReference, cached);
        }

        internal async ValueTask<ILocatorRegistryPrx?> GetLocatorRegistryAsync()
        {
            if (_locatorRegistry == null)
            {
                ILocatorRegistryPrx? registry = await Locator.GetRegistryAsync().ConfigureAwait(false);

                // The locator registry can't be located and we use ordered endpoint selection in case the locator
                // returned a proxy with some endpoints which are preferred to be tried first.
                _locatorRegistry =
                    registry?.Clone(clearLocator: true, endpointSelection: EndpointSelectionType.Ordered);
            }
            return _locatorRegistry;
        }

        private static bool CheckTTL(TimeSpan insertionTime, TimeSpan ttl) =>
            ttl == Timeout.InfiniteTimeSpan || (Time.Elapsed - insertionTime) <= ttl;

        private static void TraceDirect(string msg, Reference reference, Reference directReference)
        {
            var sb = new System.Text.StringBuilder(msg);
            sb.Append('\n');
            if (reference.Location.Count > 0)
            {
                sb.Append("protocol = ");
                sb.Append(reference.Protocol.GetName());
                sb.Append("\nlocation = ");
                sb.Append(reference.LocationAsString);
                sb.Append('\n');
            }
            else
            {
                sb.Append("well-known proxy = ");
                sb.Append(reference);
                sb.Append('\n');
            }
            sb.Append("endpoints = ");
            sb.AppendEndpointList(directReference.Endpoints);
            if (directReference.Location.Count > 0)
            {
                sb.Append("\nnew location = ");
                sb.Append(directReference.LocationAsString);
            }
            reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocatorCategory, sb.ToString());
        }

        private static void TraceInvalid(Reference reference, Reference invalidReference)
        {
            var sb = new System.Text.StringBuilder("locator returned an invalid proxy when resolving ");
            sb.Append(reference);
            sb.Append("\n received = ");
            sb.Append(invalidReference);
            reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocatorCategory, sb.ToString());
        }

        private static void TraceWellKnown(string msg, Reference wellKnown, Reference indirectReference)
        {
            Debug.Assert(wellKnown.IsWellKnown);
            var sb = new System.Text.StringBuilder(msg);
            sb.Append('\n');
            sb.Append("well-known proxy = ");
            sb.Append(wellKnown);
            sb.Append('\n');
            sb.Append("location = ");
            sb.Append(indirectReference.LocationAsString);
            wellKnown.Communicator.Logger.Trace(wellKnown.Communicator.TraceLevels.LocatorCategory, sb.ToString());
        }

        private (Reference? Reference, bool Cached) GetResolvedLocationFromCache(Reference reference, TimeSpan ttl)
        {
            if (ttl == TimeSpan.Zero) // Locator cache disabled.
            {
                return (null, false);
            }

            if (_locationCache.TryGetValue(reference, out (TimeSpan InsertionTime, Reference Reference) entry))
            {
                return (entry.Reference, CheckTTL(entry.InsertionTime, ttl));
            }
            else
            {
                return (null, false);
            }
        }

        private async Task<Reference?> ResolveLocationAsync(Reference reference, CancellationToken cancel)
        {
            if (reference.Communicator.TraceLevels.Locator > 0)
            {
                reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocatorCategory,
                    $"searching for adapter by id\nadapter = {reference.AdapterId}");
            }

            Task<Reference?>? task;
            lock (_mutex)
            {
                if (!_locationRequests.TryGetValue(reference, out task))
                {
                    // If there is no request in progress for this location, we invoke one and cache the request to
                    // prevent concurrent identical requests. It's removed once the response is received.
                    task = PerformResolveLocationAsync(reference);
                    if (!task.IsCompleted)
                    {
                        // If PerformResolveLocationAsync completed, don't add the task (it would leak since
                        // PerformResolveLocationAsync is responsible for removing it).
                        // Since PerformResolveLocationAsync locks _mutex in its finally block, the only way it can be
                        // completed now is if completed synchronously.
                        _locationRequests.Add(reference, task);
                    }
                }
            }

            return await task.WaitAsync(cancel).ConfigureAwait(false);

            async Task<Reference?> PerformResolveLocationAsync(Reference reference)
            {
                try
                {
                    IObjectPrx? proxy;

                    if (Locator.Protocol == Protocol.Ice1)
                    {
                        if (reference.Protocol != Protocol.Ice1)
                        {
                            throw new NotSupportedException("cannot resolve an ice2 proxy with an ice1 locator");
                        }

                        try
                        {
#pragma warning disable CS0618 // FindAdapterByIdAsync is deprecated
                            proxy = await Locator.FindAdapterByIdAsync(
                                reference.AdapterId,
                                cancel: CancellationToken.None).ConfigureAwait(false);
#pragma warning restore CS0618
                        }
                        catch (AdapterNotFoundException)
                        {
                            // We treat AdapterNotFoundException just like a null return value.
                            proxy = null;
                        }
                    }
                    else
                    {
                        proxy = await Locator.ResolveLocationAsync(
                            reference.Location,
                            reference.Protocol,
                            cancel: CancellationToken.None).ConfigureAwait(false);
                    }

                    if (proxy == null)
                    {
                        RemoveLocation(reference); // clear cache if it's in there.
                        return null;
                    }
                    else
                    {
                        Reference resolved = proxy.IceReference;
                        if (resolved.IsIndirect ||
                            resolved.Protocol != reference.Protocol)
                        {
                            if (reference.Communicator.TraceLevels.Locator >= 1)
                            {
                                TraceInvalid(reference, resolved);
                            }
                            return null;
                        }
                        else
                        {
                            // Cache the resolved location
                            _locationCache[reference] = (Time.Elapsed, resolved);
                            return resolved;
                        }
                    }
                }
                catch (Exception exception)
                {
                    if (reference.Communicator.TraceLevels.Locator > 0)
                    {
                        reference.Communicator.Logger.Trace(
                            reference.Communicator.TraceLevels.LocatorCategory,
                            @$"could not contact the locator to resolve location `{reference.LocationAsString
                                }'\nreason = {exception}");
                    }
                    throw;
                }
                finally
                {
                    lock (_mutex)
                    {
                        _locationRequests.Remove(reference);
                    }
                }
            }
        }

        private (Reference? Reference, bool Cached) GetResolvedWellKnownProxyFromCache(
            Reference reference,
            TimeSpan ttl)
        {
            if (ttl == TimeSpan.Zero) // Locator cache disabled.
            {
                return (null, false);
            }

            if (_wellKnownProxyCache.TryGetValue(reference, out (TimeSpan InsertionTime, Reference Reference) entry))
            {
                return (entry.Reference, CheckTTL(entry.InsertionTime, ttl));
            }
            else
            {
                return (null, false);
            }
        }

        private async Task<Reference?> ResolveWellKnownProxyAsync(Reference reference, CancellationToken cancel)
        {
            if (reference.Communicator.TraceLevels.Locator > 0)
            {
                reference.Communicator.Logger.Trace(reference.Communicator.TraceLevels.LocatorCategory,
                    $"searching for well-known object\nwell-known proxy = {reference}");
            }

            Task<Reference?>? task;
            lock (_mutex)
            {
                if (!_wellKnownProxyRequests.TryGetValue(reference, out task))
                {
                    // If there's no locator request in progress for this object, we make one and cache it to prevent
                    // making too many requests on the locator. It's removed once the locator response is received.
                    task = PerformResolveWellKnownProxyAsync(reference);
                    if (!task.IsCompleted)
                    {
                        // If PerformGetObjectProxyAsync completed, don't add the task (it would leak since
                        // PerformGetObjectProxyAsync is responsible for removing it).
                        _wellKnownProxyRequests.Add(reference, task);
                    }
                }
            }

            return await task.WaitAsync(cancel).ConfigureAwait(false);

            async Task<Reference?> PerformResolveWellKnownProxyAsync(Reference reference)
            {
                try
                {
                    IObjectPrx? proxy;

                    if (Locator.Protocol == Protocol.Ice1)
                    {
                        if (reference.Protocol != Protocol.Ice1)
                        {
                            throw new NotSupportedException("cannot resolve an ice2 proxy with an ice1 locator");
                        }

                        try
                        {
#pragma warning disable CS0618 // FindObjectByIdAsync is deprecated
                            proxy = await Locator.FindObjectByIdAsync(
                                reference.Identity,
                                cancel: CancellationToken.None).ConfigureAwait(false);
#pragma warning restore CS0618
                        }
                        catch (ObjectNotFoundException)
                        {
                            // We treat ObjectNotFoundException just like a null return value.
                            proxy = null;
                        }
                    }
                    else
                    {
                        proxy = await Locator.ResolveWellKnownProxyAsync(
                            reference.Identity,
                            reference.Protocol,
                            cancel: CancellationToken.None).ConfigureAwait(false);
                    }

                    if (proxy == null)
                    {
                        RemoveWellKnownProxy(reference); // clear cache if it's in there.
                        return null;
                    }
                    else
                    {
                        Reference resolved = proxy.IceReference;
                        if (resolved.IsWellKnown || resolved.Protocol != reference.Protocol)
                        {
                            if (reference.Communicator.TraceLevels.Locator >= 1)
                            {
                                TraceInvalid(reference, resolved);
                            }
                            return null;
                        }
                        else
                        {
                            _wellKnownProxyCache[reference] = (Time.Elapsed, resolved);
                            return resolved;
                        }
                    }
                }
                catch (Exception exception)
                {
                    if (reference.Communicator.TraceLevels.Locator > 0)
                    {
                        reference.Communicator.Logger.Trace(
                            reference.Communicator.TraceLevels.LocatorCategory,
                            @$"could not contact the locator to retrieve endpoints for well-known proxy `{reference
                                }'\nreason = {exception}");
                    }
                    throw;
                }
                finally
                {
                    lock (_mutex)
                    {
                        _wellKnownProxyRequests.Remove(reference);
                    }
                }
            }
        }

        private Reference? RemoveLocation(Reference reference) =>
            _locationCache.TryRemove(reference, out (TimeSpan InsertionTime, Reference Reference) entry) ?
                entry.Reference : null;

        private Reference? RemoveWellKnownProxy(Reference reference) =>
            _wellKnownProxyCache.TryRemove(reference,
                out (TimeSpan InsertionTime, Reference Reference) entry) ? entry.Reference : null;

        private sealed class LocationComparer : IEqualityComparer<Reference>
        {
            public bool Equals(Reference? x, Reference? y) =>
                x!.Location.SequenceEqual(y!.Location) && x.Protocol == y.Protocol;

            public int GetHashCode(Reference obj)
            {
                var hash = new HashCode();
                foreach (string s in obj.Location)
                {
                    hash.Add(s);
                }
                hash.Add(obj.Protocol);
                return hash.ToHashCode();
            }
        }

        private sealed class WellKnownProxyComparer : IEqualityComparer<Reference>
        {
            public bool Equals(Reference? x, Reference? y) =>
                x!.Identity == y!.Identity && x.Protocol == y.Protocol;

            public int GetHashCode(Reference obj) => HashCode.Combine(obj.Identity, obj.Protocol);
        }
    }
}
