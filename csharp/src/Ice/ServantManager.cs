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
        public void AddServant(Ice.IObject servant, Ice.Identity ident, string facet)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                if (facet == null)
                {
                    facet = "";
                }

                _servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.IObject> m);
                if (m == null)
                {
                    _servantMapMap[ident] = m = new Dictionary<string, Ice.IObject>();
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

        public void AddDefaultServant(Ice.IObject servant, string category)
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

        public Ice.IObject RemoveServant(Ice.Identity ident, string facet)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                if (facet == null)
                {
                    facet = "";
                }

                _servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.IObject> m);
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
                Ice.IObject obj = m[facet];
                m.Remove(facet);

                if (m.Count == 0)
                {
                    _servantMapMap.Remove(ident);
                }
                return obj;
            }
        }

        public Ice.IObject RemoveDefaultServant(string category)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                _defaultServantMap.TryGetValue(category, out Ice.IObject obj);
                if (obj == null)
                {
                    throw new Ice.NotRegisteredException("default servant", category);
                }

                _defaultServantMap.Remove(category);
                return obj;
            }
        }

        public Dictionary<string, Ice.IObject> RemoveAllFacets(Ice.Identity ident)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null);

                _servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.IObject> m);
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

        public Ice.IObject? FindServant(Ice.Identity ident, string facet)
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

                _servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.IObject> m);
                Ice.IObject? obj = null;
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

        public Ice.IObject? FindDefaultServant(string category)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.

                _defaultServantMap.TryGetValue(category, out Ice.IObject obj);
                return obj;
            }
        }

        public Dictionary<string, Ice.IObject> FindAllFacets(Ice.Identity ident)
        {
            lock (this)
            {
                Debug.Assert(_communicator != null); // Must not be called after destruction.
                return new Dictionary<string, Ice.IObject>(_servantMapMap[ident]);
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

                _servantMapMap.TryGetValue(ident, out Dictionary<string, Ice.IObject> m);
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

                _communicator = null;
            }
        }

        private Ice.Communicator? _communicator;
        private readonly string _adapterName;
        private readonly Dictionary<Ice.Identity, Dictionary<string, Ice.IObject>> _servantMapMap
                = new Dictionary<Ice.Identity, Dictionary<string, Ice.IObject>>();
        private readonly Dictionary<string, Ice.IObject> _defaultServantMap = new Dictionary<string, Ice.IObject>();
    }

}
