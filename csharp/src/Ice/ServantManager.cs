//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{

    using System.Collections.Generic;
    using System.Diagnostics;

    public sealed class ServantManager
    {
        public void addServant(Ice.Disp servant, Ice.Identity ident, string facet)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                if (facet == null)
                {
                    facet = "";
                }

                Dictionary<string, Ice.Disp> m;
                _servantMapMap.TryGetValue(ident, out m);
                if (m == null)
                {
                    _servantMapMap[ident] = (m = new Dictionary<string, Ice.Disp>());
                }
                else
                {
                    if (m.ContainsKey(facet))
                    {
                        Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                        ex.id = ident.ToString(_communicator.ToStringMode);
                        ex.kindOfObject = "servant";
                        if (facet.Length > 0)
                        {
                            ex.id += " -f " + IceUtilInternal.StringUtil.escapeString(facet, "", _communicator.ToStringMode);
                        }
                        throw ex;
                    }
                }

                m[facet] = servant;
            }
        }

        public void addDefaultServant(Ice.Disp servant, string category)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.
                if (_defaultServantMap.ContainsKey(category))
                {
                    throw new Ice.AlreadyRegisteredException("default servant", category);
                }
                _defaultServantMap[category] = servant;
            }
        }

        public Ice.Disp removeServant(Ice.Identity ident, string facet)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                if (facet == null)
                {
                    facet = "";
                }

                Dictionary<string, Ice.Disp> m;
                _servantMapMap.TryGetValue(ident, out m);
                if (m == null || !m.ContainsKey(facet))
                {
                    Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
                    ex.id = ident.ToString(_communicator.ToStringMode);
                    ex.kindOfObject = "servant";
                    if (facet.Length > 0)
                    {
                        ex.id += " -f " + IceUtilInternal.StringUtil.escapeString(facet, "", _communicator.ToStringMode);
                    }
                    throw ex;
                }
                Ice.Disp obj = m[facet];
                m.Remove(facet);

                if (m.Count == 0)
                {
                    _servantMapMap.Remove(ident);
                }
                return obj;
            }
        }

        public Ice.Disp removeDefaultServant(string category)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                Ice.Disp? obj = null;
                _defaultServantMap.TryGetValue(category, out obj);
                if (obj == null)
                {
                    throw new Ice.NotRegisteredException("default servant", category);
                }

                _defaultServantMap.Remove(category);
                return obj;
            }
        }

        public Dictionary<string, Ice.Disp> removeAllFacets(Ice.Identity ident)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null);

                Dictionary<string, Ice.Disp>? m;
                _servantMapMap.TryGetValue(ident, out m);
                if (m == null)
                {
                    Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
                    ex.id = ident.ToString(_communicator.ToStringMode);
                    ex.kindOfObject = "servant";
                    throw ex;
                }
                _servantMapMap.Remove(ident);

                return m;
            }
        }

        public Ice.Disp findServant(Ice.Identity ident, string facet)
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

                Dictionary<string, Ice.Disp> m;
                _servantMapMap.TryGetValue(ident, out m);
                Ice.Disp? obj = null;
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

        public Ice.Disp? findDefaultServant(string category)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                Ice.Disp? obj = null;
                _defaultServantMap.TryGetValue(category, out obj);
                return obj;
            }
        }

        public Dictionary<string, Ice.Disp> findAllFacets(Ice.Identity ident)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.
                return new Dictionary<string, Ice.Disp>(_servantMapMap[ident]);
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

                Dictionary<string, Ice.Disp> m;
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
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                Ice.ServantLocator l;
                _locatorMap.TryGetValue(category, out l);
                if (l != null)
                {
                    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                    ex.id = IceUtilInternal.StringUtil.escapeString(category, "", _communicator.ToStringMode);
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
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                Ice.ServantLocator l;
                _locatorMap.TryGetValue(category, out l);
                if (l == null)
                {
                    Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
                    ex.id = IceUtilInternal.StringUtil.escapeString(category, "", _communicator.ToStringMode);
                    ex.kindOfObject = "servant locator";
                    throw ex;
                }
                _locatorMap.Remove(category);
                return l;
            }
        }

        public Ice.ServantLocator findServantLocator(string category)
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

                Ice.ServantLocator result;
                _locatorMap.TryGetValue(category, out result);
                return result;
            }
        }

        //
        // Only for use by Ice.ObjectAdapterI.
        //
        public ServantManager(Ice.Communicator communicator, string adapterName)
        {
            _communicator = communicator;
            _adapterName = adapterName;
        }

        //
        // Only for use by Ice.ObjectAdapterI.
        //
        public void destroy()
        {
            Dictionary<string, Ice.ServantLocator> locatorMap = null;
            Ice.Logger logger = null;
            lock (this)
            {
                //
                // If the ServantManager has already been destroyed, we're done.
                //
                if (_communicator == null)
                {
                    return;
                }

                logger = _communicator.Logger;

                _servantMapMap.Clear();

                _defaultServantMap.Clear();

                locatorMap = new Dictionary<string, Ice.ServantLocator>(_locatorMap);
                _locatorMap.Clear();

                _communicator = null;
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

        private Ice.Communicator _communicator;
        private readonly string _adapterName;
        private Dictionary<Ice.Identity, Dictionary<string, Ice.Disp>> _servantMapMap
                = new Dictionary<Ice.Identity, Dictionary<string, Ice.Disp>>();
        private Dictionary<string, Ice.Disp> _defaultServantMap = new Dictionary<string, Ice.Disp>();
        private Dictionary<string, Ice.ServantLocator> _locatorMap = new Dictionary<string, Ice.ServantLocator>();
    }

}
