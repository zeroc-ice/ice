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
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        final int sz = _collectorFactories.size();
        for (int i = 0; i < sz; i++)
        {
            IceInternal.CollectorFactory factory =
                (IceInternal.CollectorFactory)_collectorFactories.elementAt(i);
            factory.activate();
        }
    }

    public synchronized void
    hold()
    {
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        final int sz = _collectorFactories.size();
        for (int i = 0; i < sz; i++)
        {
            IceInternal.CollectorFactory factory =
                (IceInternal.CollectorFactory)_collectorFactories.elementAt(i);
            factory.hold();
        }
    }

    public synchronized void
    deactivate()
    {
        if (_collectorFactories.isEmpty())
        {
            //
            // Ignore deactivation requests if the Object Adapter has
            // already been deactivated.
            //
            return;
        }

        int sz = _collectorFactories.size();
        for (int i = 0; i < sz; i++)
        {
            IceInternal.CollectorFactory factory =
                (IceInternal.CollectorFactory)_collectorFactories.elementAt(i);
            factory.destroy();
        }
        _collectorFactories.clear();

        _activeServantMap.clear();
        // TODO: Hint

        int sz = _locatorMap.size();
        java.util.Set valueSet = _locatorMap.valueSet();
        ServantLocator[] locators = new ServantLocator[sz];
        valueSet.toArray(locators);
        for (int i = 0; i < sz; i++)
        {
            locators[i].deactivate();
        }
        _locatorMap.clear();
        // TODO: Hint
    }

    public synchronized ObjectPrx
    add(Ice.Object servant, String ident)
    {
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        _activeServantMap.put(ident, object);

        return newProxy(ident);
    }

    public synchronized ObjectPrx
    addTemporary(Ice.Object servant)
    {
        if (_collectorFactories.isEmpty())
        {
            throw new ObjectAdapterDeactivatedException();
        }

        String ident = TODO

        _activeServantMap.put(ident, object);

        return newProxy(ident);
    }

    public synchronized void
    remove(String identity)
    {
    }

    public synchronized void
    addServantLocator(ServantLocator locator, String prefix)
    {
    }

    public synchronized void
    removeServantLocator(String prefix)
    {
    }

    public synchronized ServantLocator
    findServantLocator(String prefix)
    {
    }

    public synchronized Ice.Object
    identityToServant(String identity)
    {
    }

    public Ice.Object
    proxyToServant(ObjectPrx proxy)
    {
    }

    public synchronized ObjectPrx
    createProxy(String identity)
    {
    }

    //
    // Only for use by IceInternal.ObjectAdapterFactory
    //
    public
    ObjectAdapterI(IceInternal.Instance instance, String, String)
    {
    }

    private ObjectPrx
    newProxy(String)
    {
    }

    private boolean
    isLocal(ObjectPrx)
    {
    }

    IceInternal.Instance _instance;
    String _name;
    // TODO: Better collection type?
    java.util.Vector _collectorFactories = new java.util.Vector();
    java.util.HashMap _activeServantMap = new java.util.HashMap();
    // TODO: Hint
    java.util.HashMap _locatorMap = new java.util.HashMap();
    // TODO: Hint
}
