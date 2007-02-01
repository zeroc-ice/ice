// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class LocatorInfo
{
    LocatorInfo(Ice.LocatorPrx locator, LocatorTable table)
    {
        _locator = locator;
        _table = table;
    }

    synchronized public void
    destroy()
    {
        _locatorRegistry = null;
        _table.clear();
    }

    public boolean
    equals(java.lang.Object obj)
    {
        if(this == obj)
        {
            return true;
        }

        if(obj instanceof LocatorInfo)
        {
            return _locator.equals(((LocatorInfo)obj)._locator);
        }

        return false;
    }

    public Ice.LocatorPrx
    getLocator()
    {
        //
        // No synchronization necessary, _locator is immutable.
        //
        return _locator;
    }

    public synchronized Ice.LocatorRegistryPrx
    getLocatorRegistry()
    {
        if(_locatorRegistry == null) // Lazy initialization
        {
            _locatorRegistry = _locator.getRegistry();

            //
            // The locator registry can't be located.
            //
            _locatorRegistry = Ice.LocatorRegistryPrxHelper.uncheckedCast(_locatorRegistry.ice_locator(null));
        }
        
        return _locatorRegistry;
    }

    public EndpointI[]
    getEndpoints(IndirectReference ref, int ttl, Ice.BooleanHolder cached)
    {
        EndpointI[] endpoints = null;
        Ice.ObjectPrx object = null;
        cached.value = true;
        String adapterId = ref.getAdapterId();
        Ice.Identity identity = ref.getIdentity();
        
        try
        {
            if(adapterId.length() > 0)
            {
                endpoints = _table.getAdapterEndpoints(adapterId, ttl);
                if(endpoints == null)
                {
                    cached.value = false;
                    
                    if(ref.getInstance().traceLevels().location >= 1)
                    {
                        StringBuffer s = new StringBuffer();
                        s.append("searching for adapter by id\n");
                        s.append("adapter = " + adapterId);
                        ref.getInstance().initializationData().logger.trace(
                            ref.getInstance().traceLevels().locationCat, s.toString());
                    }

                    //
                    // Search the adapter in the location service if we didn't
                    // find it in the cache.
                    //
                    object = _locator.findAdapterById(adapterId);
                    if(object != null)
                    {
                        endpoints = ((Ice.ObjectPrxHelperBase)object).__reference().getEndpoints();
                        
                        if(endpoints.length > 0)
                        {
                            _table.addAdapterEndpoints(adapterId, endpoints);
                        }
                    }
                }
            }
            else
            {
                boolean objectCached = true;
                object = _table.getProxy(identity, ttl);
                if(object == null)
                {
                    objectCached = false;

                    if(ref.getInstance().traceLevels().location >= 1)
                    {
                        StringBuffer s = new StringBuffer();
                        s.append("searching for object by id\n");
                        s.append("object = " + ref.getInstance().identityToString(identity));
                        ref.getInstance().initializationData().logger.trace(
                            ref.getInstance().traceLevels().locationCat, s.toString());
                    }

                    object = _locator.findObjectById(identity);
                }
                
                boolean endpointsCached = true;
                if(object != null)
                {
                    Reference r = ((Ice.ObjectPrxHelperBase)object).__reference();
                    if(r instanceof DirectReference)
                    {
                        endpointsCached = false;
                        DirectReference odr = (DirectReference)r;
                        endpoints = odr.getEndpoints();
                    }
                    else
                    {
                        IndirectReference oir = (IndirectReference)r;
                        if(oir.getAdapterId().length() > 0)
                        {
                            Ice.BooleanHolder c = new Ice.BooleanHolder();
                            endpoints = getEndpoints(oir, ttl, c);
                            endpointsCached = c.value;
                        }
                    }
                }
                
                if(!objectCached && endpoints != null && endpoints.length > 0)
                {
                    _table.addProxy(identity, object);
                }

                cached.value = objectCached || endpointsCached;
            }
        }
        catch(Ice.AdapterNotFoundException ex)
        {
            if(ref.getInstance().traceLevels().location >= 1)
            {
                StringBuffer s = new StringBuffer();
                s.append("adapter not found\n");
                s.append("adapter = " + adapterId);
                ref.getInstance().initializationData().logger.trace(
                    ref.getInstance().traceLevels().locationCat, s.toString());
            }

            Ice.NotRegisteredException e = new Ice.NotRegisteredException();
            e.kindOfObject = "object adapter";
            e.id = adapterId;
            throw e;
        }
        catch(Ice.ObjectNotFoundException ex)
        {
            if(ref.getInstance().traceLevels().location >= 1)
            {
                StringBuffer s = new StringBuffer();
                s.append("object not found\n");
                s.append("object = " + ref.getInstance().identityToString(identity));
                ref.getInstance().initializationData().logger.trace(
                    ref.getInstance().traceLevels().locationCat, s.toString());
            }

            Ice.NotRegisteredException e = new Ice.NotRegisteredException();
            e.kindOfObject = "object";
            e.id = ref.getInstance().identityToString(identity);
            throw e;
        }
        catch(Ice.NotRegisteredException ex)
        {
            throw ex;
        }
        catch(Ice.LocalException ex)
        {
            if(ref.getInstance().traceLevels().location >= 1)
            {
                StringBuffer s = new StringBuffer();
                s.append("couldn't contact the locator to retrieve adapter endpoints\n");
                if(adapterId.length() > 0)
                {
                    s.append("adapter = " + adapterId + "\n");
                }
                else
                {
                    s.append("object = " + ref.getInstance().identityToString(identity) + "\n");
                }
                s.append("reason = " + ex);
                ref.getInstance().initializationData().logger.trace(
                    ref.getInstance().traceLevels().locationCat, s.toString());
            }
            throw ex;
        }

        if(ref.getInstance().traceLevels().location >= 1)
        {
            if(endpoints != null && endpoints.length > 0)
            {
                if(cached.value)
                {
                    trace("found endpoints in locator table", ref, endpoints);
                }
                else
                {
                    trace("retrieved endpoints from locator, adding to locator table", ref, endpoints);
                }
            }
            else
            {
                StringBuffer s = new StringBuffer();
                s.append("no endpoints configured for ");
                if(adapterId.length() > 0)
                {
                    s.append("adapter\n");
                    s.append("adapter = " + adapterId + "\n");
                }
                else
                {
                    s.append("object\n");
                    s.append("object = " + ref.getInstance().identityToString(identity) + "\n");
                }
            }
        }

        return endpoints == null ? new EndpointI[0] : endpoints;
    }

    public void
    clearObjectCache(IndirectReference ref)
    {
        if(ref.getAdapterId().length() == 0)
        {
            Ice.ObjectPrx object = _table.removeProxy(ref.getIdentity());
            if(object != null)
            {
                if(((Ice.ObjectPrxHelperBase)object).__reference() instanceof IndirectReference)
                {
                    IndirectReference oir = (IndirectReference)((Ice.ObjectPrxHelperBase)object).__reference();
                    if(oir.getAdapterId().length() > 0)
                    {
                        clearCache(oir);
                    }
                }
                else
                {
                    if(ref.getInstance().traceLevels().location >= 2)
                    {
                        trace("removed endpoints from locator table",
                              ref, ((Ice.ObjectPrxHelperBase)object).__reference().getEndpoints());
                    }
                }
            }
        }
    }
    
    public void
    clearCache(IndirectReference ref)
    {
        if(ref.getAdapterId().length() > 0)
        {
            EndpointI[] endpoints = _table.removeAdapterEndpoints(ref.getAdapterId());

            if(endpoints != null && ref.getInstance().traceLevels().location >= 2)
            {
                trace("removed endpoints from locator table\n", ref, endpoints);
            }
        }
        else
        {
            Ice.ObjectPrx object = _table.removeProxy(ref.getIdentity());
            if(object != null)
            {
                if(((Ice.ObjectPrxHelperBase)object).__reference() instanceof IndirectReference)
                {
                    IndirectReference oir = (IndirectReference)((Ice.ObjectPrxHelperBase)object).__reference();
                    if(oir.getAdapterId().length() > 0)
                    {
                        clearCache(oir);
                    }
                }
                else
                {
                    if(ref.getInstance().traceLevels().location >= 2)
                    {
                        trace("removed endpoints from locator table",
                              ref, ((Ice.ObjectPrxHelperBase)object).__reference().getEndpoints());
                    }
                }
            }
        }
    }

    private void
    trace(String msg, IndirectReference ref, EndpointI[] endpoints)
    {
        StringBuffer s = new StringBuffer();
        s.append(msg + "\n");
        if(ref.getAdapterId().length() > 0)
        {
            s.append("adapter = " + ref.getAdapterId() + "\n");
        }
        else
        {
            s.append("object = " + ref.getInstance().identityToString(ref.getIdentity()) + "\n");
        }

        s.append("endpoints = ");
        final int sz = endpoints.length;
        for(int i = 0; i < sz; i++)
        {
            s.append(endpoints[i].toString());
            if(i + 1 < sz)
                s.append(":");
        }

        ref.getInstance().initializationData().logger.trace(ref.getInstance().traceLevels().locationCat, s.toString());
    }

    private final Ice.LocatorPrx _locator;
    private Ice.LocatorRegistryPrx _locatorRegistry;
    private final LocatorTable _table;
}

