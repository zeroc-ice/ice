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
        
        if (_incomingConnectionFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        java.util.ListIterator i = _incomingConnectionFactories.listIterator();
        while (i.hasNext())
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)i.next();
            factory.activate();
        }
    }

    public synchronized void
    hold()
    {
        if (_incomingConnectionFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        java.util.ListIterator i = _incomingConnectionFactories.listIterator();
        while (i.hasNext())
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)i.next();
            factory.hold();
        }
    }

    public synchronized void
    deactivate()
    {
        if (_incomingConnectionFactories.isEmpty())
        {
            //
            // Ignore deactivation requests if the Object Adapter has
            // already been deactivated.
            //
            return;
        }

        java.util.ListIterator i = _incomingConnectionFactories.listIterator();
        while (i.hasNext())
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)i.next();
            factory.destroy();
        }
        _incomingConnectionFactories.clear();

        _activeServantMap.clear();

        java.util.Iterator p = _locatorMap.values().iterator();
        while (p.hasNext())
        {
            ServantLocator locator = (ServantLocator)p.next();
            locator.deactivate();
        }
        _locatorMap.clear();
    }

    public synchronized ObjectPrx
    add(Ice.Object servant, Identity ident)
    {
        if (_incomingConnectionFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        _activeServantMap.put(ident, servant);

        return newProxy(ident);
    }

    public synchronized ObjectPrx
    addTemporary(Ice.Object servant)
    {
        if (_incomingConnectionFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        long now = System.currentTimeMillis();
        Identity ident = new Identity();
        ident.category = "";
        ident.name = "." + Long.toHexString(now / 1000L) + "." +
            Long.toHexString(now % 1000L) + "." +
            Integer.toHexString(_rand.nextInt());

        _activeServantMap.put(ident, servant);

        return newProxy(ident);
    }

    public synchronized void
    remove(Identity ident)
    {
        if (_incomingConnectionFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        _activeServantMap.remove(ident);
    }

    public synchronized void
    addServantLocator(ServantLocator locator, String prefix)
    {
        if (_incomingConnectionFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        _locatorMap.put(prefix, locator);
    }

    public synchronized void
    removeServantLocator(String prefix)
    {
        if (_incomingConnectionFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        ServantLocator locator = (ServantLocator)_locatorMap.remove(prefix);
        if (locator != null)
        {
            locator.deactivate();
        }
    }

    public synchronized ServantLocator
    findServantLocator(String prefix)
    {
        if (_incomingConnectionFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        return (ServantLocator)_locatorMap.get(prefix);
    }

    public synchronized Ice.Object
    identityToServant(Identity ident)
    {
        return (Ice.Object)_activeServantMap.get(ident);
    }

    public Ice.Object
    proxyToServant(ObjectPrx proxy)
    {
        IceInternal.Reference ref = ((ObjectPrxHelper)proxy).__reference();
        return identityToServant(ref.identity);
    }

    public synchronized ObjectPrx
    createProxy(Identity ident)
    {
        if (_incomingConnectionFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        return newProxy(ident);
    }

    //
    // Only for use by IceInternal.ObjectAdapterFactory
    //
    public
    ObjectAdapterI(IceInternal.Instance instance, String name, String endpts)
    {
        _instance = instance;
        _name = name;

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
                // Don't store the endpoint in the adapter. The Connection
                // might change it, for example, to fill in the real port
                // number if a zero port number is given.
                //
                IceInternal.Endpoint endp =
                    IceInternal.Endpoint.endpointFromString(instance, es);
                _incomingConnectionFactories.add(
                    new IceInternal.IncomingConnectionFactory(instance, endp,
                                                              this));

                if (end == s.length())
                {
                    break;
                }

                beg = end + 1;
            }
        }
        catch (LocalException ex)
        {
            if (!_incomingConnectionFactories.isEmpty())
            {
                deactivate();
            }

            throw ex;
        }

        if (_incomingConnectionFactories.isEmpty())
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
        catch (NumberFormatException ex)
        {
            // TODO: Do anything?
        }
    }

    protected void
    finalize()
        throws Throwable
    {
        if (!_incomingConnectionFactories.isEmpty())
        {
            deactivate();
        }

        super.finalize();
    }

    private ObjectPrx
    newProxy(Identity ident)
    {
        IceInternal.Endpoint[] endpoints =
            new IceInternal.Endpoint[_incomingConnectionFactories.size()];
        int n = 0;
        java.util.ListIterator i = _incomingConnectionFactories.listIterator();
        while (i.hasNext())
        {
            IceInternal.IncomingConnectionFactory factory =
                (IceInternal.IncomingConnectionFactory)i.next();
            endpoints[n++] = factory.endpoint();
        }

        IceInternal.Reference reference = new IceInternal.Reference(_instance,
            ident, "", IceInternal.Reference.ModeTwoway, false, endpoints,
            endpoints);
        return _instance.proxyFactory().referenceToProxy(reference);
    }

    public boolean
    isLocal(ObjectPrx proxy)
    {
        // TODO: Optimize?
        IceInternal.Reference ref = ((ObjectPrxHelper)proxy).__reference();
        IceInternal.Endpoint[] endpoints = ref.endpoints;
        for (int n = 0; n < endpoints.length; n++)
        {
            java.util.ListIterator i =
                _incomingConnectionFactories.listIterator();
            while (i.hasNext())
            {
                IceInternal.IncomingConnectionFactory factory =
                    (IceInternal.IncomingConnectionFactory)i.next();
                if (factory.equivalent(endpoints[n]))
                {
                    return true;
                }
            }
        }

        return false;
    }

    private IceInternal.Instance _instance;
    private String _name;
    private java.util.LinkedList _incomingConnectionFactories =
        new java.util.LinkedList();
    private java.util.HashMap _activeServantMap = new java.util.HashMap();
    private java.util.HashMap _locatorMap = new java.util.HashMap();
    private java.util.Random _rand = new java.util.Random();

}
