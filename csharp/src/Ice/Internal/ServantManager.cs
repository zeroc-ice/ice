// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Diagnostics;

namespace Ice.Internal;

public sealed class ServantManager : Object
{
    public void addServant(Ice.Object servant, Ice.Identity ident, string facet)
    {
        lock (this)
        {
            Debug.Assert(_instance is not null); // Must not be called after destruction.

            facet ??= "";

            if (_servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.Object>? m))
            {
                if (m.ContainsKey(facet))
                {
                    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                    ex.id = Ice.Util.identityToString(ident, _instance.toStringMode());
                    ex.kindOfObject = "servant";
                    if (facet.Length > 0)
                    {
                        ex.id += " -f " + Ice.UtilInternal.StringUtil.escapeString(facet, "", _instance.toStringMode());
                    }
                    throw ex;
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

    public void addDefaultServant(Ice.Object servant, string category)
    {
        lock (this)
        {
            Debug.Assert(_instance is not null); // Must not be called after destruction.
            if (_defaultServantMap.TryGetValue(category, out Ice.Object? obj))
            {
                Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                ex.kindOfObject = "default servant";
                ex.id = category;
                throw ex;
            }

            _defaultServantMap[category] = servant;
        }
    }

    public Ice.Object removeServant(Ice.Identity ident, string facet)
    {
        lock (this)
        {
            Debug.Assert(_instance is not null); // Must not be called after destruction.

            facet ??= "";

            Dictionary<string, Ice.Object>? m = null;
            _servantMapMap.TryGetValue(ident, out m);
            Ice.Object? obj = null;
            if (m is null || !m.TryGetValue(facet, out Ice.Object? value))
            {
                Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
                ex.id = Ice.Util.identityToString(ident, _instance.toStringMode());
                ex.kindOfObject = "servant";
                if (facet.Length > 0)
                {
                    ex.id += " -f " + Ice.UtilInternal.StringUtil.escapeString(facet, "", _instance.toStringMode());
                }
                throw ex;
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

    public Ice.Object removeDefaultServant(string category)
    {
        lock (this)
        {
            Debug.Assert(_instance is not null); // Must not be called after destruction.

            if (!_defaultServantMap.TryGetValue(category, out Ice.Object? obj))
            {
                Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
                ex.kindOfObject = "default servant";
                ex.id = category;
                throw ex;
            }

            _defaultServantMap.Remove(category);
            return obj;
        }
    }

    public Dictionary<string, Ice.Object> removeAllFacets(Ice.Identity ident)
    {
        lock (this)
        {
            Debug.Assert(_instance is not null);

            if (!_servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.Object>? m))
            {
                Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
                ex.id = Ice.Util.identityToString(ident, _instance.toStringMode());
                ex.kindOfObject = "servant";
                throw ex;
            }
            _servantMapMap.Remove(ident);

            return m;
        }
    }

    public Ice.Object? findServant(Ice.Identity ident, string facet)
    {
        lock (this)
        {
            //
            // This assert is not valid if the adapter dispatch incoming
            // requests from bidir connections. This method might be called if
            // requests are received over the bidir connection after the
            // adapter was deactivated.
            //
            //Debug.Assert(_instance is not null); // Must not be called after destruction.

            facet ??= "";

            Ice.Object? obj = null;
            if (_servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.Object>? m))
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

    public Ice.Object? findDefaultServant(string category)
    {
        lock (this)
        {
            Debug.Assert(_instance is not null); // Must not be called after destruction.
            return _defaultServantMap.TryGetValue(category, out Ice.Object? obj) ? obj : null;
        }
    }

    public Dictionary<string, Ice.Object> findAllFacets(Ice.Identity ident)
    {
        lock (this)
        {
            Debug.Assert(_instance is not null); // Must not be called after destruction.

            Dictionary<string, Ice.Object>? m = _servantMapMap[ident];
            if (m is not null)
            {
                return new Dictionary<string, Ice.Object>(m);
            }

            return new Dictionary<string, Ice.Object>();
        }
    }

    public bool hasServant(Ice.Identity ident)
    {
        lock (this)
        {
            //
            // This assert is not valid if the adapter dispatch incoming
            // requests from bidir connections. This method might be called if
            // requests are received over the bidir connection after the
            // adapter was deactivated.
            //
            //
            //Debug.Assert(_instance is not null); // Must not be called after destruction.

            return _servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.Object>? m) ? m.Count != 0 : false;
        }
    }

    public void addServantLocator(Ice.ServantLocator locator, string category)
    {
        lock (this)
        {
            Debug.Assert(_instance is not null); // Must not be called after destruction.

            if (_locatorMap.TryGetValue(category, out Ice.ServantLocator? l))
            {
                Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                ex.id = Ice.UtilInternal.StringUtil.escapeString(category, "", _instance.toStringMode());
                ex.kindOfObject = "servant locator";
                throw ex;
            }

            _locatorMap[category] = locator;
        }
    }

    public Ice.ServantLocator removeServantLocator(string category)
    {
        lock (this)
        {
            Debug.Assert(_instance is not null); // Must not be called after destruction.

            if (!_locatorMap.TryGetValue(category, out Ice.ServantLocator? l))
            {
                Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
                ex.id = Ice.UtilInternal.StringUtil.escapeString(category, "", _instance.toStringMode());
                ex.kindOfObject = "servant locator";
                throw ex;
            }
            _locatorMap.Remove(category);
            return l;
        }
    }

    public Ice.ServantLocator? findServantLocator(string category)
    {
        lock (this)
        {
            //
            // This assert is not valid if the adapter dispatch incoming
            // requests from bidir connections. This method might be called if
            // requests are received over the bidir connection after the
            // adapter was deactivated.
            //
            //
            //Debug.Assert(_instance is not null); // Must not be called after destruction.

            return _locatorMap.TryGetValue(category, out Ice.ServantLocator? result) ? result : null;
        }
    }

    //
    // Only for use by Ice.ObjectAdapter.
    //
    public ServantManager(Instance instance, string adapterName)
    {
        _instance = instance;
        _adapterName = adapterName;
    }

    //
    // Only for use by Ice.ObjectAdapter.
    //
    public void destroy()
    {
        Dictionary<string, Ice.ServantLocator>? locatorMap = null;
        Ice.Logger? logger = null;
        lock (this)
        {
            //
            // If the ServantManager has already been destroyed, we're done.
            //
            if (_instance is null)
            {
                return;
            }

            logger = _instance.initializationData().logger!;

            _servantMapMap.Clear();

            _defaultServantMap.Clear();

            locatorMap = new Dictionary<string, Ice.ServantLocator>(_locatorMap);
            _locatorMap.Clear();

            _instance = null;
        }

        foreach (KeyValuePair<string, Ice.ServantLocator> p in locatorMap)
        {
            Ice.ServantLocator locator = p.Value;
            try
            {
                locator.deactivate(p.Key);
            }
            catch (System.Exception ex)
            {
                string s = "exception during locator deactivation:\n" + "object adapter: `"
                            + _adapterName + "'\n" + "locator category: `" + p.Key + "'\n" + ex;
                logger.error(s);
            }
        }
    }

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

    private Instance? _instance;
    private readonly string _adapterName;
    private Dictionary<Ice.Identity, Dictionary<string, Ice.Object>> _servantMapMap = new();
    private Dictionary<string, Ice.Object> _defaultServantMap = new();
    private Dictionary<string, Ice.ServantLocator> _locatorMap = new();
}
