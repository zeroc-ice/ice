// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.evictor;
import test.Freeze.evictor.Test.*;

public final class RemoteEvictorI extends _RemoteEvictorDisp
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static class Initializer implements Freeze.ServantInitializer
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
                ServantI servantImpl =  (ServantI) ((_ServantTie) servant).ice_delegate();
                servantImpl.init(_remoteEvictor, _evictor);
            }
            else
            {
                ServantI servantImpl =  (ServantI) ((_FacetTie) servant).ice_delegate();
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

    RemoteEvictorI(Ice.Communicator communicator, String envName, String category, boolean transactional)
    {
        _envName = envName;
        _category = category;

        //
        // NOTE: COMPILERBUG: The timeout here is required for OS X. It shouldn't be too low since
        // some operations can take some time to complete on slow machines.
        //
        _evictorAdapter = communicator.
            createObjectAdapterWithEndpoints(java.util.UUID.randomUUID().toString(), "default -t 60000");

        Initializer initializer = new Initializer();

        if(transactional)
        {
            _evictor = Freeze.Util.createTransactionalEvictor(_evictorAdapter, envName, category, null, initializer,
                                                              null, true);
        }
        else
        {
            _evictor = Freeze.Util.createBackgroundSaveEvictor(_evictorAdapter, envName, category, initializer, null,
                                                               true);
        }

        //
        // Check that we can get an iterator on a non-existing facet
        //
        Freeze.EvictorIterator p = _evictor.getIterator("foo", 1);
        test(p.hasNext() == false);

        initializer.init(this, _evictor);

        _evictorAdapter.addServantLocator(_evictor, category);
        _evictorAdapter.activate();
    }

    public void
    setSize(int size, Ice.Current current)
    {
        _evictor.setSize(size);
    }

    public ServantPrx
    createServant(String id, int value, Ice.Current current)
        throws AlreadyRegisteredException, EvictorDeactivatedException
    {
        Ice.Identity ident = new Ice.Identity();
        ident.category = _category;
        ident.name = id;
        _ServantTie tie = new _ServantTie();
        tie.ice_delegate(new ServantI(tie, this, _evictor, value));
        try
        {
            return ServantPrxHelper.uncheckedCast(_evictor.add(tie, ident));
        }
        catch(Ice.AlreadyRegisteredException e)
        {
            throw new AlreadyRegisteredException();
        }
        catch(Ice.ObjectAdapterDeactivatedException e)
        {
            throw new EvictorDeactivatedException();
        }
        catch(Freeze.EvictorDeactivatedException e)
        {
            throw new EvictorDeactivatedException();
        }
        catch(Ice.LocalException e)
        {
            System.out.println("Throwing " + e.toString());
            throw e;
        }
    }

    public ServantPrx
    getServant(String id, Ice.Current current)
    {
        Ice.Identity ident = new Ice.Identity();
        ident.category = _category;
        ident.name = id;
        return ServantPrxHelper.uncheckedCast(_evictorAdapter.createProxy(ident));
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
        current.adapter.remove(current.adapter.getCommunicator().stringToIdentity(_category));
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

    private final String _envName;
    private String _category;
    private Freeze.Evictor _evictor;
    private Ice.ObjectAdapter _evictorAdapter;
}
