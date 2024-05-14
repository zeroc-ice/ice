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
            Debug.Assert(_instance != null); // Must not be called after destruction.

            facet ??= "";

            _servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.Object>? m);
            if (m == null)
            {
                _servantMapMap[ident] = (m = new Dictionary<string, Ice.Object>());
            }
            else
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

            m[facet] = servant;
        }
    }

    public void addDefaultServant(Ice.Object servant, string category)
    {
        lock (this)
        {
            Debug.Assert(_instance != null); // Must not be called after destruction.
            _defaultServantMap.TryGetValue(category, out Ice.Object? obj);
            if (obj != null)
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
            Debug.Assert(_instance != null); // Must not be called after destruction.

           facet ??= "";

            Dictionary<string, Ice.Object>? m;
            _servantMapMap.TryGetValue(ident, out m);
            Ice.Object? obj = null;
            if (m == null || !m.TryGetValue(facet, out Ice.Object? value))
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
            Debug.Assert(_instance != null); // Must not be called after destruction.

            Ice.Object? obj = null;
            _defaultServantMap.TryGetValue(category, out obj);
            if (obj == null)
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
            Debug.Assert(_instance != null);

            Dictionary<string, Ice.Object>? m;
            _servantMapMap.TryGetValue(ident, out m);
            if (m == null)
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
            //Debug.Assert(_instance != null); // Must not be called after destruction.

            if (facet == null)
            {
                facet = "";
            }

            Dictionary<string, Ice.Object>? m;
            _servantMapMap.TryGetValue(ident, out m);
            Ice.Object? obj = null;
            if (m == null)
            {
                _defaultServantMap.TryGetValue(ident.category, out obj);
                if (obj == null)
                {
                    _defaultServantMap.TryGetValue("", out obj);
                }
            }
            else
            {
                m.TryGetValue(facet, out obj);
            }

            return obj;
        }
    }

    public Ice.Object? findDefaultServant(string category)
    {
        lock (this)
        {
            Debug.Assert(_instance != null); // Must not be called after destruction.

            Ice.Object? obj = null;
            _defaultServantMap.TryGetValue(category, out obj);
            return obj;
        }
    }

    public Dictionary<string, Ice.Object> findAllFacets(Ice.Identity ident)
    {
        lock (this)
        {
            Debug.Assert(_instance != null); // Must not be called after destruction.

            Dictionary<string, Ice.Object> m = _servantMapMap[ident];
            if (m != null)
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
            //Debug.Assert(_instance != null); // Must not be called after destruction.

            Dictionary<string, Ice.Object>? m;
            _servantMapMap.TryGetValue(ident, out m);
            if (m == null)
            {
                return false;
            }
            else
            {
                Debug.Assert(m.Count != 0);
                return true;
            }
        }
    }

    public void addServantLocator(Ice.ServantLocator locator, string category)
    {
        lock (this)
        {
            Debug.Assert(_instance != null); // Must not be called after destruction.

            Ice.ServantLocator? l;
            _locatorMap.TryGetValue(category, out l);
            if (l != null)
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
            Debug.Assert(_instance != null); // Must not be called after destruction.

            Ice.ServantLocator? l;
            _locatorMap.TryGetValue(category, out l);
            if (l is null)
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
            //Debug.Assert(_instance != null); // Must not be called after destruction.

            Ice.ServantLocator? result;
            _locatorMap.TryGetValue(category, out result);
            return result;
        }
    }

    //
    // Only for use by Ice.ObjectAdapterI.
    //
    public ServantManager(Instance instance, string adapterName)
    {
        _instance = instance;
        _adapterName = adapterName;
    }

    //
    // Only for use by Ice.ObjectAdapterI.
    //
    public void destroy()
    {
        Dictionary<string, Ice.ServantLocator> ?locatorMap = null;
        Ice.Logger? logger = null;
        lock (this)
        {
            //
            // If the ServantManager has already been destroyed, we're done.
            //
            if (_instance == null)
            {
                return;
            }

            logger = _instance.initializationData().logger;

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
