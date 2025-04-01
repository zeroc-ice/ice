// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Diagnostics;

namespace Ice.Internal;

internal sealed class ServantManager : Object
{
    private readonly Instance _instance;
    private readonly string _adapterName;
    private readonly Dictionary<Identity, Dictionary<string, Object>> _servantMapMap = [];
    private readonly Dictionary<string, Object> _defaultServantMap = [];
    private readonly Dictionary<string, ServantLocator> _locatorMap = [];
    private bool _isDestroyed;
    private readonly object _mutex = new();

    public async ValueTask<OutgoingResponse> dispatchAsync(IncomingRequest request)
    {
        Current current = request.current;
        Object? servant = findServant(current.id, current.facet);

        if (servant is not null)
        {
            // the simple, common path
            return await servant.dispatchAsync(request).ConfigureAwait(false);
        }

        // Else, check servant locators
        ServantLocator? locator = findServantLocator(current.id.category);
        if (locator is null && current.id.category.Length > 0)
        {
            locator = findServantLocator("");
        }

        if (locator is not null)
        {
            object? cookie;

            try
            {
                servant = locator.locate(current, out cookie);
            }
            catch
            {
                // Skip the encapsulation. This allows the next batch requests in the same InputStream to proceed.
                request.inputStream.skipEncapsulation();
                throw;
            }

            if (servant is not null)
            {
                try
                {
                    return await servant.dispatchAsync(request).ConfigureAwait(false);
                }
                finally
                {
                    locator.finished(current, servant, cookie);
                }
            }
        }

        Debug.Assert(servant is null);

        // Skip the encapsulation. This allows the next batch requests in the same InputStream to proceed.
        request.inputStream.skipEncapsulation();
        if (hasServant(current.id))
        {
            throw new FacetNotExistException();
        }
        else
        {
            throw new ObjectNotExistException();
        }
    }

    internal void addServant(Object servant, Identity ident, string facet)
    {
        lock (_mutex)
        {
            facet ??= "";

            if (_servantMapMap.TryGetValue(ident, out Dictionary<string, Object>? m))
            {
                if (m.ContainsKey(facet))
                {
                    string id = Ice.Util.identityToString(ident, _instance.toStringMode());
                    if (facet.Length > 0)
                    {
                        id += " -f " + UtilInternal.StringUtil.escapeString(facet, "", _instance.toStringMode());
                    }
                    throw new AlreadyRegisteredException("servant", id);
                }
            }
            else
            {
                m = new();
                _servantMapMap[ident] = m;
            }

            m[facet] = servant;
        }
    }

    internal void addDefaultServant(Object servant, string category)
    {
        lock (_mutex)
        {
            if (_defaultServantMap.TryGetValue(category, out Object? obj))
            {
                throw new AlreadyRegisteredException("default servant", category);
            }

            _defaultServantMap[category] = servant;
        }
    }

    internal Object removeServant(Identity ident, string facet)
    {
        lock (_mutex)
        {
            facet ??= "";

            _servantMapMap.TryGetValue(ident, out Dictionary<string, Object>? m);
            Object? obj = null;
            if (m is null || !m.TryGetValue(facet, out Object? value))
            {
                string id = Ice.Util.identityToString(ident, _instance.toStringMode());
                if (facet.Length > 0)
                {
                    id += " -f " + UtilInternal.StringUtil.escapeString(facet, "", _instance.toStringMode());
                }
                throw new NotRegisteredException("servant", id);
            }
            obj = value;
            m.Remove(facet);

            if (m.Count == 0)
            {
                _servantMapMap.Remove(ident);
            }
            return obj;
        }
    }

    internal Object removeDefaultServant(string category)
    {
        lock (_mutex)
        {
            if (!_defaultServantMap.TryGetValue(category, out Object? obj))
            {
                throw new NotRegisteredException("default servant", category);
            }

            _defaultServantMap.Remove(category);
            return obj;
        }
    }

    internal Dictionary<string, Object> removeAllFacets(Identity ident)
    {
        lock (_mutex)
        {
            if (!_servantMapMap.TryGetValue(ident, out Dictionary<string, Object>? m))
            {
                throw new NotRegisteredException(
                    "servant",
                    Ice.Util.identityToString(ident, _instance.toStringMode()));
            }
            _servantMapMap.Remove(ident);

            return m;
        }
    }

    internal Object? findServant(Identity ident, string facet)
    {
        lock (_mutex)
        {
            facet ??= "";

            Object? obj = null;
            if (_servantMapMap.TryGetValue(ident, out Dictionary<string, Object>? m))
            {
                m.TryGetValue(facet, out obj);
            }
            else
            {
                _defaultServantMap.TryGetValue(ident.category, out obj);
                if (obj is null)
                {
                    _defaultServantMap.TryGetValue("", out obj);
                }
            }

            return obj;
        }
    }

    internal Object? findDefaultServant(string category)
    {
        lock (_mutex)
        {
            return _defaultServantMap.TryGetValue(category, out Object? obj) ? obj : null;
        }
    }

    internal Dictionary<string, Object> findAllFacets(Identity ident)
    {
        lock (_mutex)
        {
            Dictionary<string, Object>? m = _servantMapMap[ident];
            if (m is not null)
            {
                return new Dictionary<string, Object>(m);
            }

            return new Dictionary<string, Object>();
        }
    }

    internal bool hasServant(Identity ident)
    {
        lock (_mutex)
        {
            return _servantMapMap.TryGetValue(ident, out Dictionary<string, Object>? m) ? m.Count != 0 : false;
        }
    }

    internal void addServantLocator(ServantLocator locator, string category)
    {
        lock (_mutex)
        {
            if (_locatorMap.TryGetValue(category, out ServantLocator? l))
            {
                throw new AlreadyRegisteredException(
                    "servant locator",
                    UtilInternal.StringUtil.escapeString(category, "", _instance.toStringMode()));
            }

            _locatorMap[category] = locator;
        }
    }

    internal ServantLocator removeServantLocator(string category)
    {
        lock (_mutex)
        {
            if (!_locatorMap.TryGetValue(category, out ServantLocator? l))
            {
                throw new NotRegisteredException(
                    "servant locator",
                    UtilInternal.StringUtil.escapeString(category, "", _instance.toStringMode()));
            }
            _locatorMap.Remove(category);
            return l;
        }
    }

    internal ServantLocator? findServantLocator(string category)
    {
        lock (_mutex)
        {
            return _locatorMap.TryGetValue(category, out ServantLocator? result) ? result : null;
        }
    }

    //
    // Only for use by ObjectAdapter.
    //
    internal ServantManager(Instance instance, string adapterName)
    {
        _instance = instance;
        _adapterName = adapterName;
    }

    //
    // Only for use by ObjectAdapter.
    //
    internal void destroy()
    {
        Dictionary<string, ServantLocator> locatorMap;
        lock (_mutex)
        {
            if (_isDestroyed)
            {
                return;
            }

            _isDestroyed = true;
            _servantMapMap.Clear();
            _defaultServantMap.Clear();

            locatorMap = new Dictionary<string, ServantLocator>(_locatorMap);
            _locatorMap.Clear();
        }

        foreach (KeyValuePair<string, ServantLocator> p in locatorMap)
        {
            ServantLocator locator = p.Value;
            try
            {
                locator.deactivate(p.Key);
            }
            catch (System.Exception ex)
            {
                string s = "exception during locator deactivation:\n" + "object adapter: `"
                            + _adapterName + "'\n" + "locator category: `" + p.Key + "'\n" + ex;
                _instance.initializationData().logger!.error(s);
            }
        }
    }
}
