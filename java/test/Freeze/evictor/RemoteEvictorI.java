// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public final class RemoteEvictorI extends Test._RemoteEvictorDisp
{
    RemoteEvictorI(Ice.ObjectAdapter adapter, String category, Freeze.Evictor evictor)
    {
        _adapter = adapter;
        _category = category;
        _evictor = evictor;
        Ice.Communicator communicator = adapter.getCommunicator();
        _evictorAdapter = communicator.createObjectAdapterWithEndpoints(Ice.Util.generateUUID(), "default");
        _evictorAdapter.addServantLocator(evictor, category);
        _evictorAdapter.activate();
    }

    public void
    setSize(int size, Ice.Current current)
    {
        _evictor.setSize(size);
    }

    public Test.ServantPrx
    createServant(int id, int value, Ice.Current current)
    {
        Ice.Identity ident = new Ice.Identity();
        ident.category = _category;
        ident.name = "" + id;
	Test._ServantTie tie = new Test._ServantTie();
	tie.ice_delegate(new ServantI(tie, this, _evictor, value));
        _evictor.createObject(ident, tie);
        return Test.ServantPrxHelper.uncheckedCast(_evictorAdapter.createProxy(ident));
    }

    public Test.ServantPrx
    getServant(int id, Ice.Current current)
    {
        Ice.Identity ident = new Ice.Identity();
        ident.category = _category;
        ident.name = "" + id;
        return Test.ServantPrxHelper.uncheckedCast(_evictorAdapter.createProxy(ident));
    }

    public void
    deactivate(Ice.Current current)
    {
        _evictorAdapter.deactivate();
        _evictorAdapter.waitForDeactivate();
        _adapter.remove(Ice.Util.stringToIdentity(_category));
    }
    
    public void
    destroyAllServants(Ice.Current current)
    {
	//
	// Only for test purpose: don't use such a small value in 
	// a real application!
	//
	int batchSize = 2;

	Freeze.EvictorIterator p = _evictor.getIterator(batchSize, false);
	while(p.hasNext())
	{
	    _evictor.destroyObject((Ice.Identity) p.next());
	}
    }

    private Ice.ObjectAdapter _adapter;
    private String _category;
    private Freeze.Evictor _evictor;
    private Ice.ObjectAdapter _evictorAdapter;
}
