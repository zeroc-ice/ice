// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CompletionStage;

final class ServantManager implements Object {
    private Instance _instance;
    private final String _adapterName;
    private final Map<Identity, Map<String, Object>> _servantMapMap =
        new HashMap<Identity, Map<String, Object>>();
    private final Map<String, Object> _defaultServantMap = new HashMap<String, Object>();
    private final Map<String, ServantLocator> _locatorMap = new HashMap<String, ServantLocator>();

    @Override
    public CompletionStage<OutgoingResponse> dispatch(IncomingRequest request)
        throws UserException {
        final Current current = request.current;
        Object servant = findServant(current.id, current.facet);

        if (servant != null) {
            // the simple, common path
            return servant.dispatch(request);
        }

        // Else, check servant locators
        ServantLocator locator = findServantLocator(current.id.category);
        if (locator == null && current.id.category.length() > 0) {
            locator = findServantLocator("");
        }

        java.lang.Object cookie = null;

        if (locator != null) {
            boolean skipEncapsulation = true;
            try {
                ServantLocator.LocateResult locateResult = locator.locate(current);
                servant = locateResult.returnValue;
                cookie = locateResult.cookie;
                skipEncapsulation = false;
            } finally {
                if (skipEncapsulation) {
                    // Skip the encapsulation on exception. This allows the next batch requests in
                    // the same InputStream to proceed.
                    request.inputStream.skipEncapsulation();
                }
            }
        }

        if (servant != null) {
            CompletionStage<OutgoingResponse> response;
            try {
                response = servant.dispatch(request);
            } catch (UserException | RuntimeException | Error exception) {
                // We catch Error because ServantLocator guarantees finished gets called no matter
                // what.
                locator.finished(current, servant, cookie);
                throw exception; // unless finished above throws another exception
            }

            final Object servantFinal = servant;
            final ServantLocator locatorFinal = locator;
            final java.lang.Object cookieFinal = cookie;
            return response.handle(
                (r, ex) -> {
                    try {
                        locatorFinal.finished(current, servantFinal, cookieFinal);
                    } catch (UserException finishedEx) {
                        ex = finishedEx;
                    }
                    if (ex != null) {
                        // We only marshal errors and runtime exceptions (including
                        // CompletionException) at a higher level.
                        if (ex instanceof Error errorEx) {
                            throw errorEx;
                        }
                        if (ex instanceof RuntimeException runtimeEx) {
                            throw runtimeEx;
                        }
                        return current.createOutgoingResponse(ex);
                    } else {
                        return r;
                    }
                });
        } else {
            // Skip the encapsulation. This allows the next batch requests in the same InputStream
            // to proceed.
            request.inputStream.skipEncapsulation();
            if (hasServant(current.id)) {
                throw new FacetNotExistException();
            } else {
                throw new ObjectNotExistException();
            }
        }
    }

    public synchronized void addServant(Object servant, Identity ident, String facet) {
        assert _instance != null; // Must not be called after destruction.

        if (facet == null) {
            facet = "";
        }

        Map<String, Object> m = _servantMapMap.get(ident);
        if (m == null) {
            m = new HashMap<String, Object>();
            _servantMapMap.put(ident, m);
        } else {
            if (m.containsKey(facet)) {
                String id = Util.identityToString(ident, _instance.toStringMode());
                if (!facet.isEmpty()) {
                    id += " -f " + StringUtil.escapeString(facet, "", _instance.toStringMode());
                }
                throw new AlreadyRegisteredException("servant", id);
            }
        }

        m.put(facet, servant);
    }

    public synchronized void addDefaultServant(Object servant, String category) {
        assert (_instance != null); // Must not be called after destruction

        Object obj = _defaultServantMap.get(category);
        if (obj != null) {
            throw new AlreadyRegisteredException("default servant", category);
        }

        _defaultServantMap.put(category, servant);
    }

    public synchronized Object removeServant(Identity ident, String facet) {
        assert (_instance != null); // Must not be called after destruction.

        if (facet == null) {
            facet = "";
        }

        Map<String, Object> m = _servantMapMap.get(ident);
        Object obj = null;
        if (m == null || (obj = m.remove(facet)) == null) {
            String id = Util.identityToString(ident, _instance.toStringMode());
            if (!facet.isEmpty()) {
                id += " -f " + StringUtil.escapeString(facet, "", _instance.toStringMode());
            }
            throw new NotRegisteredException("servant", id);
        }

        if (m.isEmpty()) {
            _servantMapMap.remove(ident);
        }
        return obj;
    }

    public synchronized Object removeDefaultServant(String category) {
        assert (_instance != null); // Must not be called after destruction.

        Object obj = _defaultServantMap.get(category);
        if (obj == null) {
            throw new NotRegisteredException("default servant", category);
        }

        _defaultServantMap.remove(category);
        return obj;
    }

    public synchronized Map<String, Object> removeAllFacets(Identity ident) {
        assert (_instance != null); // Must not be called after destruction.

        Map<String, Object> m = _servantMapMap.get(ident);
        if (m == null) {
            final String id = Util.identityToString(ident, _instance.toStringMode());
            throw new NotRegisteredException("servant", id);
        }

        _servantMapMap.remove(ident);

        return m;
    }

    public synchronized Object findServant(Identity ident, String facet) {
        //
        // This assert is not valid if the adapter dispatch incoming
        // requests from bidir connections. This method might be called if
        // requests are received over the bidir connection after the
        // adapter was deactivated.
        //
        // assert(_instance != null); // Must not be called after destruction.

        if (facet == null) {
            facet = "";
        }

        Map<String, Object> m = _servantMapMap.get(ident);
        Object obj = null;
        if (m == null) {
            obj = _defaultServantMap.get(ident.category);
            if (obj == null) {
                obj = _defaultServantMap.get("");
            }
        } else {
            obj = m.get(facet);
        }

        return obj;
    }

    public synchronized Object findDefaultServant(String category) {
        assert (_instance != null); // Must not be called after destruction.

        return _defaultServantMap.get(category);
    }

    public synchronized Map<String, Object> findAllFacets(Identity ident) {
        assert (_instance != null); // Must not be called after destruction.

        Map<String, Object> m = _servantMapMap.get(ident);
        if (m != null) {
            return new HashMap<String, Object>(m);
        }

        return new HashMap<String, Object>();
    }

    public synchronized boolean hasServant(Identity ident) {
        //
        // This assert is not valid if the adapter dispatch incoming
        // requests from bidir connections. This method might be called if
        // requests are received over the bidir connection after the
        // adapter was deactivated.
        //
        // assert(_instance != null); // Must not be called after destruction.

        Map<String, Object> m = _servantMapMap.get(ident);
        if (m == null) {
            return false;
        } else {
            assert (!m.isEmpty());
            return true;
        }
    }

    public synchronized void addServantLocator(ServantLocator locator, String category) {
        assert (_instance != null); // Must not be called after destruction.

        ServantLocator l = _locatorMap.get(category);
        if (l != null) {
            final String id = StringUtil.escapeString(category, "", _instance.toStringMode());
            throw new AlreadyRegisteredException("servant locator", id);
        }

        _locatorMap.put(category, locator);
    }

    public synchronized ServantLocator removeServantLocator(String category) {
        ServantLocator l = null;
        assert (_instance != null); // Must not be called after destruction.

        l = _locatorMap.remove(category);
        if (l == null) {
            final String id = StringUtil.escapeString(category, "", _instance.toStringMode());
            throw new NotRegisteredException("servant locator", id);
        }
        return l;
    }

    public synchronized ServantLocator findServantLocator(String category) {
        //
        // This assert is not valid if the adapter dispatch incoming
        // requests from bidir connections. This method might be called if
        // requests are received over the bidir connection after the
        // adapter was deactivated.
        //
        // assert(_instance != null); // Must not be called after destruction.

        return _locatorMap.get(category);
    }

    //
    // Only for use by ObjectAdapter.
    //
    public ServantManager(Instance instance, String adapterName) {
        _instance = instance;
        _adapterName = adapterName;
    }

    //
    // Only for use by ObjectAdapter.
    //
    public void destroy() {
        Map<String, ServantLocator> locatorMap = new HashMap<String, ServantLocator>();
        Logger logger = null;
        synchronized (this) {
            //
            // If the ServantManager has already been destroyed, we're done.
            //
            if (_instance == null) {
                return;
            }

            logger = _instance.initializationData().logger;

            _servantMapMap.clear();

            _defaultServantMap.clear();

            locatorMap.putAll(_locatorMap);
            _locatorMap.clear();

            _instance = null;
        }

        for (Map.Entry<String, ServantLocator> p : locatorMap.entrySet()) {
            ServantLocator locator = p.getValue();
            try {
                locator.deactivate(p.getKey());
            } catch (Exception ex) {
                String s =
                    "exception during locator deactivation:\n"
                        + "object adapter: `"
                        + _adapterName
                        + "'\n"
                        + "locator category: `"
                        + p.getKey()
                        + "'\n"
                        + Ex.toString(ex);
                logger.error(s);
            }
        }
    }
}
