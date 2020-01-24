//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System;

namespace IceInternal
{
    public sealed class ServantManager
    {
        public void AddServant(Ice.Disp servant, Ice.Identity ident, string facet)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                if (facet == null)
                {
                    facet = "";
                }

                _servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.Disp> m);
                if (m == null)
                {
                    _servantMapMap[ident] = m = new Dictionary<string, Ice.Disp>();
                }
                else
                {
                    if (m.ContainsKey(facet))
                    {
                        throw new ArgumentException(
                            $"Servant `{ident.ToString(_communicator.ToStringMode)}' already has a facet named `{facet}'",
                            nameof(facet));
                    }
                }

                m[facet] = servant;
            }
        }

        public void AddDefaultServant(Ice.Disp servant, string category)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.
                if (_defaultServantMap.ContainsKey(category))
                {
                    throw new ArgumentException($"A default servant for category `{category}' is already registered",
                                                nameof(category));
                }
                _defaultServantMap[category] = servant;
            }
        }

        public Ice.Disp RemoveServant(Ice.Identity ident, string facet)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                if (facet == null)
                {
                    facet = "";
                }

                _servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.Disp> m);
                if (m == null || !m.ContainsKey(facet))
                {
                    var ex = new Ice.NotRegisteredException();
                    ex.Id = ident.ToString(_communicator.ToStringMode);
                    ex.KindOfObject = "servant";
                    if (facet.Length > 0)
                    {
                        ex.Id += " -f " + IceUtilInternal.StringUtil.escapeString(facet, "", _communicator.ToStringMode);
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

        public Ice.Disp RemoveDefaultServant(string category)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                _defaultServantMap.TryGetValue(category, out Ice.Disp obj);
                if (obj == null)
                {
                    throw new Ice.NotRegisteredException("default servant", category);
                }

                _defaultServantMap.Remove(category);
                return obj;
            }
        }

        public Dictionary<string, Ice.Disp> RemoveAllFacets(Ice.Identity ident)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null);

                _servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.Disp> m);
                if (m == null)
                {
                    var ex = new Ice.NotRegisteredException();
                    ex.Id = ident.ToString(_communicator.ToStringMode);
                    ex.KindOfObject = "servant";
                    throw ex;
                }
                _servantMapMap.Remove(ident);

                return m;
            }
        }

        public Ice.Disp FindServant(Ice.Identity ident, string facet)
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

                _servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.Disp> m);
                Ice.Disp? obj = null;
                if (m == null)
                {
                    _defaultServantMap.TryGetValue(ident.Category, out obj);
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

        public Ice.Disp? FindDefaultServant(string category)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                _defaultServantMap.TryGetValue(category, out Ice.Disp obj);
                return obj;
            }
        }

        public Dictionary<string, Ice.Disp> FindAllFacets(Ice.Identity ident)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.
                return new Dictionary<string, Ice.Disp>(_servantMapMap[ident]);
            }
        }

        public bool HasServant(Ice.Identity ident)
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

                _servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.Disp> m);
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

        public void AddServantLocator(Ice.IServantLocator locator, string category)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                if (_locatorMap.ContainsKey(category))
                {
                    throw new ArgumentException($"A servant locator for category {category} is already registered", nameof(category));
                }

                _locatorMap[category] = locator;
            }
        }

        public Ice.IServantLocator RemoveServantLocator(string category)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                _locatorMap.TryGetValue(category, out Ice.IServantLocator l);
                if (l == null)
                {
                    var ex = new Ice.NotRegisteredException();
                    ex.Id = IceUtilInternal.StringUtil.escapeString(category, "", _communicator.ToStringMode);
                    ex.KindOfObject = "servant locator";
                    throw ex;
                }
                _locatorMap.Remove(category);
                return l;
            }
        }

        public Ice.IServantLocator FindServantLocator(string category)
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

                _locatorMap.TryGetValue(category, out Ice.IServantLocator result);
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
        public void Destroy()
        {
            Dictionary<string, Ice.IServantLocator> locatorMap;
            Ice.ILogger logger;
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

                locatorMap = new Dictionary<string, Ice.IServantLocator>(_locatorMap);
                _locatorMap.Clear();

                _communicator = null;
            }

            foreach (KeyValuePair<string, Ice.IServantLocator> p in locatorMap)
            {
                Ice.IServantLocator locator = p.Value;
                try
                {
                    locator.Deactivate(p.Key);
                }
                catch (System.Exception ex)
                {
                    string s = "exception during locator deactivation:\n" + "object adapter: `"
                                + _adapterName + "'\n" + "locator category: `" + p.Key + "'\n" + ex;
                    logger.Error(s);
                }
            }
        }

        private Ice.Communicator? _communicator;
        private readonly string _adapterName;
        private readonly Dictionary<Ice.Identity, Dictionary<string, Ice.Disp>> _servantMapMap
                = new Dictionary<Ice.Identity, Dictionary<string, Ice.Disp>>();
        private readonly Dictionary<string, Ice.Disp> _defaultServantMap = new Dictionary<string, Ice.Disp>();
        private readonly Dictionary<string, Ice.IServantLocator> _locatorMap = new Dictionary<string, Ice.IServantLocator>();
    }

}
