// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public class ObjectAdapterI implements ObjectAdapter
{
    public String
    getName()
    {
        return _name; // _name is immutable
    }

    public Communicator
    getCommunicator()
    {
        return _instance.communicator(); // _instance is immutable
    }

    public synchronized void
    activate()
    {
        /*
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        java.util.ListIterator i = _collectorFactories.listIterator();
        while (i.hasNext())
        {
            IceInternal.CollectorFactory factory =
                (IceInternal.CollectorFactory)i.next();
            factory.activate();
        }
        */
    }

    public synchronized void
    hold()
    {
        /*
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        java.util.ListIterator i = _collectorFactories.listIterator();
        while (i.hasNext())
        {
            IceInternal.CollectorFactory factory =
                (IceInternal.CollectorFactory)i.next();
            factory.hold();
        }
        */
    }

    public synchronized void
    deactivate()
    {
        /*
        if (_collectorFactories.isEmpty())
        {
            //
            // Ignore deactivation requests if the Object Adapter has
            // already been deactivated.
            //
            return;
        }

        java.util.ListIterator i = _collectorFactories.listIterator();
        while (i.hasNext())
        {
            IceInternal.CollectorFactory factory =
                (IceInternal.CollectorFactory)i.next();
            factory.destroy();
        }
        _collectorFactories.clear();

        _activeServantMap.clear();
        // TODO: Hint

        int sz = _locatorMap.size();
        java.util.Set entrySet = _locatorMap.entrySet();
        ServantLocator[] locators = new ServantLocator[sz];
        entrySet.toArray(locators);
        for (int n = 0; n < sz; n++)
        {
            locators[n].deactivate();
        }
        _locatorMap.clear();
        // TODO: Hint
        */
    }

    public synchronized ObjectPrx
    add(Ice.Object servant, String ident)
    {
        /*
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        _activeServantMap.put(ident, servant);

        return newProxy(ident);
        */
        return null;
    }

    public synchronized ObjectPrx
    addTemporary(Ice.Object servant)
    {
        /*
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        long now = System.currentTimeMillis();
        String ident = "." + (now / 1000L) + "." + (now % 1000L) + "." +
            _rand.nextInt();

        _activeServantMap.put(ident, servant);

        return newProxy(ident);
        */
        return null;
    }

    public synchronized void
    remove(String ident)
    {
        /*
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        _activeServantMap.remove(ident);
        */
    }

    public synchronized void
    addServantLocator(ServantLocator locator, String prefix)
    {
        /*
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        _locatorMap.put(prefix, locator);
        */
    }

    public synchronized void
    removeServantLocator(String prefix)
    {
        /*
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        ServantLocator locator = (ServantLocator)_locatorMap.remove(prefix);
        if (locator != null)
        {
            locator.deactivate();
        }
        */
    }

    public synchronized ServantLocator
    findServantLocator(String prefix)
    {
        /*
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        return (ServantLocator)_locatorMap.get(prefix);
        */
        return null;
    }

    public synchronized Ice.Object
    identityToServant(String ident)
    {
        /*
        return (Ice.Object)_activeServantMap.get(ident);
        */
        return null;
    }

    public Ice.Object
    proxyToServant(ObjectPrx proxy)
    {
        /*
        IceInternal.Reference ref = proxy.__reference();
        return identityToServant(ref.identity);
        */
        return null;
    }

    public synchronized ObjectPrx
    createProxy(String ident)
    {
        /*
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        return newProxy(ident);
        */
        return null;
    }

    //
    // Only for use by IceInternal.ObjectAdapterFactory
    //
    public
    ObjectAdapterI(IceInternal.Instance instance, String name, String endpts)
    {
        _instance = instance;
        _name = name;

        /*
        String s = endpts.toLowerCase();

        int beg = 0;
        int end;

        try
        {
            while (true)
            {
                end = s.indexOf(':', beg);
                if (end == -1)
                {
                    end = s.length();
                }

                if (end == beg)
                {
                    throw new EndpointParseException();
                }

                String es = s.substring(beg, end);

                //
                // Don't store the endpoint in the adapter. The Collector
                // might change it, for example, to fill in the real port
                // number if a zero port number is given.
                //
                IceInternal.Endpoint endp =
                    IceInternal.Endpoint.endpointFromString(es);
                _collectorFactories.add(
                    new IceInternal.CollectorFactory(instance, this, endp));

                if (end == s.length())
                {
                    break;
                }

                beg = end + 1;
            }
        }
        catch(LocalException ex)
        {
            if (!_collectorFactories.isEmpty())
            {
                deactivate();
            }

            throw ex;
        }

        if (_collectorFactories.isEmpty())
        {
            throw new EndpointParseException();
        }

        try
        {
            String value =
                _instance.properties().getProperty("Ice.PrintAdapterReady");
            if (Integer.parseInt(value) >= 1)
            {
                System.out.println(_name + " readY");
            }
        }
        catch(NumberFormatException ex)
        {
            // TODO: Do anything?
        }
        */
    }

    protected void
    finalize()
        throws Throwable
    {
        /*
        if (!_collectorFactories.isEmpty())
        {
            deactivate();
        }

        super.finalize();
        */
    }

    private ObjectPrx
    newProxy(String ident)
    {
        /*
        IceInternal.Endpoint[] endpoints =
            new IceInternal.Endpoint[_collectorFactories.size()];
        int n = 0;
        java.util.ListIterator i = _collectorFactories.listIterator();
        while (i.hasNext())
        {
            IceInternal.CollectorFactory factory =
                (IceInternal.CollectorFactory)i.next();
            endpoints[n++] = factory.endpoint();
        }

        IceInternal.Reference reference = new IceInternal.Reference(_instance,
            ident, "", IceInternal.Reference.ModeTwoway, false, endpoints,
            endpoints);
        return _instance.proxyFactory().referenceToProxy(reference);
        */
        return null;
    }

    public boolean
    isLocal(ObjectPrx proxy)
    {
        /*
        // TODO: Optimize?
        IceInternal.Reference ref = proxy.__reference();
        IceInternal.Endpoint[] endpoints = ref.endpoints;
        for (int n = 0; n < endpoints.length; n++)
        {
            java.util.ListIterator i = _collectorFactories.listIterator();
            while (i.hasNext())
            {
                IceInternal.CollectorFactory factory =
                    (IceInternal.CollectorFactory)i.next();
                if (factory.equivalent(endpoints[n]))
                {
                    return true;
                }
            }
        }

        return false;
        */
        return false;
    }

    private IceInternal.Instance _instance;
    private String _name;
    /*
    private java.util.LinkedList _collectorFactories =
        new java.util.LinkedList();
    private java.util.HashMap _activeServantMap = new java.util.HashMap();
    // TODO: Hint
    private java.util.HashMap _locatorMap = new java.util.HashMap();
    // TODO: Hint
    private java.util.Random _rand = new java.util.Random();
    */

}
