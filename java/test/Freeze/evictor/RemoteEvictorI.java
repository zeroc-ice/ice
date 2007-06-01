// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class RemoteEvictorI extends Test._RemoteEvictorDisp
{
    static class Initializer extends Ice.LocalObjectImpl implements Freeze.ServantInitializer
    {
        public void
        initialize(Ice.ObjectAdapter adapter, Ice.Identity ident, String facet, Ice.Object servant)
        {
            if(servant instanceof AccountI)
            {
                AccountI account = (AccountI)servant;
                account.init((Freeze.TransactionalEvictor)_evictor);
            }
            else if(facet.length() == 0)
            {
                ServantI servantImpl =  (ServantI) ((Test._ServantTie) servant).ice_delegate();
                servantImpl.init(_remoteEvictor, _evictor);
            }
            else
            {
                ServantI servantImpl =  (ServantI) ((Test._FacetTie) servant).ice_delegate();
                servantImpl.init(_remoteEvictor, _evictor);
            }
        }

        void init(RemoteEvictorI remoteEvictor, Freeze.Evictor evictor)
        {
            _remoteEvictor = remoteEvictor;
            _evictor = evictor;
        }

        private RemoteEvictorI _remoteEvictor;
        private Freeze.Evictor _evictor;
    }


    RemoteEvictorI(Ice.ObjectAdapter adapter, String envName, String category, boolean transactional)
    {
        _adapter = adapter;
        _envName = envName;
        _category = category;

        //
        // NOTE: COMPILERBUG: The timeout here is required for MacOS X.
        //
        _evictorAdapter = _adapter.getCommunicator().
            createObjectAdapterWithEndpoints(Ice.Util.generateUUID(), "default -t 30000");

        Initializer initializer = new Initializer();

        if(transactional)
        {
            _evictor = Freeze.Util.createTransactionalEvictor(_evictorAdapter, envName, category, null, initializer, null, true);
        }
        else
        {
            _evictor = Freeze.Util.createBackgroundSaveEvictor(_evictorAdapter, envName, category, initializer, null, true);
        }

        initializer.init(this, _evictor);
        
        _evictorAdapter.addServantLocator(_evictor, category);
        _evictorAdapter.activate();
    }

    public void
    setSize(int size, Ice.Current current)
    {
        _evictor.setSize(size);
    }

    public Test.ServantPrx
    createServant(String id, int value, Ice.Current current)
        throws Test.AlreadyRegisteredException, Test.EvictorDeactivatedException
    {
        Ice.Identity ident = new Ice.Identity();
        ident.category = _category;
        ident.name = id;
        Test._ServantTie tie = new Test._ServantTie();
        tie.ice_delegate(new ServantI(tie, this, _evictor, value));
        try
        {
            return Test.ServantPrxHelper.uncheckedCast(_evictor.add(tie, ident));
        }
        catch(Ice.AlreadyRegisteredException e)
        {
            throw new Test.AlreadyRegisteredException();
        }
        catch(Ice.ObjectAdapterDeactivatedException e)
        {
            throw new Test.EvictorDeactivatedException();
        }
        catch(Freeze.EvictorDeactivatedException e)
        {
            throw new Test.EvictorDeactivatedException();
        }
        catch(Ice.LocalException e)
        {
            System.out.println("Throwing " + e.toString());
            throw e;
        }
    }

    public Test.ServantPrx
    getServant(String id, Ice.Current current)
    {
        Ice.Identity ident = new Ice.Identity();
        ident.category = _category;
        ident.name = id;
        return Test.ServantPrxHelper.uncheckedCast(_evictorAdapter.createProxy(ident));
    }

    public void
    saveNow(Ice.Current current)
    {
        if(_evictor instanceof Freeze.BackgroundSaveEvictor)
        {
            _evictor.getIterator("", 1);
        }
        //
        // Otherwise everything is always saved
        //
    }

    public void
    deactivate(Ice.Current current)
    {
        _evictorAdapter.destroy();
        _adapter.remove(_adapter.getCommunicator().stringToIdentity(_category));
    }
    
    public void
    destroyAllServants(String facet, Ice.Current current)
    {
        //
        // Only for test purpose: don't use such a small value in 
        // a real application!
        //
        int batchSize = 2;

        Freeze.EvictorIterator p = _evictor.getIterator(facet, batchSize);
        while(p.hasNext())
        {
            _evictor.remove((Ice.Identity) p.next());
        }
    }

    final public String 
    envName()
    {
        return _envName;
    }    


    private Ice.ObjectAdapter _adapter;
    private final String _envName;
    private String _category;
    private Freeze.Evictor _evictor;
    private Ice.ObjectAdapter _evictorAdapter;
}
