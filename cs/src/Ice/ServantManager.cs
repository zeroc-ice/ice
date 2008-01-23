// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

using System.Collections.Generic;
using System.Diagnostics;

public sealed class ServantManager
{
    public void addServant(Ice.Object servant, Ice.Identity ident, string facet)
    {
        lock(this)
        {
            Debug.Assert(instance_ != null); // Must not be called after destruction.
            
            if(facet == null)
            {
                facet = "";
            }

            Dictionary<string, Ice.Object> m;
            _servantMapMap.TryGetValue(ident, out m);
            if(m == null)
            {
                _servantMapMap[ident] = (m = new Dictionary<string, Ice.Object>());
            }
            else
            {
                if(m.ContainsKey(facet))
                {
                    Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                    ex.id = instance_.identityToString(ident);
                    ex.kindOfObject = "servant";
                    if(facet.Length > 0)
                    {
                        ex.id += " -f " + IceUtilInternal.StringUtil.escapeString(facet, "");
                    }
                    throw ex;
                }
            }
            
            m[facet] = servant;
        }
    }
    
    public Ice.Object removeServant(Ice.Identity ident, string facet)
    {
        lock(this)
        {
            Debug.Assert(instance_ != null); // Must not be called after destruction.
            
            if(facet == null)
            {
                facet = "";
            }

            Dictionary<string, Ice.Object> m;
            _servantMapMap.TryGetValue(ident, out m);
            Ice.Object obj = null;
            if(m == null || !m.ContainsKey(facet))
            {
                Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
                ex.id = Ice.Util.identityToString(ident);
                ex.kindOfObject = "servant";
                if(facet.Length > 0)
                {
                    ex.id += " -f " + IceUtilInternal.StringUtil.escapeString(facet, "");
                }
                throw ex;
            }
            obj = m[facet];
            m.Remove(facet);
            
            if(m.Count == 0)
            {
                _servantMapMap.Remove(ident);
            }
            return obj;
        }
    }
    
    public Dictionary<string, Ice.Object> removeAllFacets(Ice.Identity ident)
    {
        lock(this)
        {
            Debug.Assert(instance_ != null);

            Dictionary<string, Ice.Object> m;
            _servantMapMap.TryGetValue(ident, out m);
            if(m == null)
            {
                Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
                ex.id = Ice.Util.identityToString(ident);
                ex.kindOfObject = "servant";
                throw ex;
            }
            _servantMapMap.Remove(ident);

            return m;
        }
    }

    public Ice.Object findServant(Ice.Identity ident, string facet)
    {
        lock(this)
        {
            //
            // This assert is not valid if the adapter dispatch incoming
            // requests from bidir connections. This method might be called if
            // requests are received over the bidir connection after the
            // adapter was deactivated.
            //  
            //Debug.Assert(instance_ != null); // Must not be called after destruction.
            
            if(facet == null)
            {
                facet = "";
            }

            Dictionary<string, Ice.Object> m;
            _servantMapMap.TryGetValue(ident, out m);
            Ice.Object obj = null;
            if(m != null)
            {
                m.TryGetValue(facet, out obj);
            }

            return obj;
        }
    }

    public Dictionary<string, Ice.Object> findAllFacets(Ice.Identity ident)
    {
        lock(this)
        {
            Debug.Assert(instance_ != null); // Must not be called after destruction.

            Dictionary<string, Ice.Object> m = _servantMapMap[ident];
            if(m != null)
            {
                return new Dictionary<string, Ice.Object>(m);
            }

            return new Dictionary<string, Ice.Object>();
        }
    }

    public bool hasServant(Ice.Identity ident)
    {
        lock(this)
        {
            //
            // This assert is not valid if the adapter dispatch incoming
            // requests from bidir connections. This method might be called if
            // requests are received over the bidir connection after the
            // adapter was deactivated.
            //  
            //
            //Debug.Assert(instance_ != null); // Must not be called after destruction.

            Dictionary<string, Ice.Object> m;
            _servantMapMap.TryGetValue(ident, out m);
            if(m == null)
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
        lock(this)
        {
            Debug.Assert(instance_ != null); // Must not be called after destruction.
            
            Ice.ServantLocator l;
            _locatorMap.TryGetValue(category, out l);
            if(l != null)
            {
                Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
                ex.id = IceUtilInternal.StringUtil.escapeString(category, "");
                ex.kindOfObject = "servant locator";
                throw ex;
            }
            
            _locatorMap[category] = locator;
        }
    }
    
    public Ice.ServantLocator findServantLocator(string category)
    {
        lock(this)
        {
            //
            // This assert is not valid if the adapter dispatch incoming
            // requests from bidir connections. This method might be called if
            // requests are received over the bidir connection after the
            // adapter was deactivated.
            //  
            //
            //Debug.Assert(instance_ != null); // Must not be called after destruction.
            
            Ice.ServantLocator result;
            _locatorMap.TryGetValue(category, out result);
            return result;
        }
    }
    
    //
    // Only for use by Ice.ObjectAdapterI.
    //
    public ServantManager(Instance instance, string adapterName)
    {
        instance_ = instance;
        _adapterName = adapterName;
    }
    
    /*
    ~ServantManager()
    {
        //
        // Don't check whether destroy() has been called. It might have
        // not been called if the associated object adapter was not
        // properly deactivated.
        //
        //lock(this)
        //{
            //IceUtil.Assert.FinalizerAssert(instance_ == null);
        //}
    }
    */
    
    //
    // Only for use by Ice.ObjectAdapterI.
    //
    public void destroy()
    {
        lock(this)
        {
            Debug.Assert(instance_ != null); // Must not be called after destruction.
            
            _servantMapMap.Clear();
            
            foreach(KeyValuePair<string, Ice.ServantLocator> p in _locatorMap)
            {
                Ice.ServantLocator locator = p.Value;
                try
                {
                    locator.deactivate(p.Key);
                }
                catch(System.Exception ex)
                {
                    string s = "exception during locator deactivation:\n" + "object adapter: `"
                               + _adapterName + "'\n" + "locator category: `" + p.Key + "'\n" + ex;
                    instance_.initializationData().logger.error(s);
                }
            }
            
            _locatorMap.Clear();

            instance_ = null;
        }
    }
    
    private Instance instance_;
    private readonly string _adapterName;
    private Dictionary <Ice.Identity, Dictionary<string, Ice.Object>> _servantMapMap
            = new Dictionary<Ice.Identity, Dictionary<string, Ice.Object>>();
    private Dictionary<string, Ice.ServantLocator> _locatorMap = new Dictionary<string, Ice.ServantLocator>();
}

}
