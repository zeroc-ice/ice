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
        _lastSavedValue = -1;
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
	Server.ServantTie tie = new Server.ServantTie();
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

    public int
    getLastSavedValue(Ice.Current current)
    {
        int result = _lastSavedValue;
        return result;
    }

    public void
    clearLastSavedValue(Ice.Current current)
    {
        _lastSavedValue = -1;
    }

    public
    void saveNow(Ice.Current current)
    {
	_evictor.saveNow();
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
	Freeze.EvictorIterator p = _evictor.getIterator();
	while(p.hasNext())
	{
	    _evictor.destroyObject(p.next());
	}
	p.destroy();
	_evictor.saveNow();
    }

    void
    setLastSavedValue(int value)
    {
        _lastSavedValue = value;
    }

    private Ice.ObjectAdapter _adapter;
    private String _category;
    private Freeze.Evictor _evictor;
    private Ice.ObjectAdapter _evictorAdapter;
    private int _lastSavedValue;
}
