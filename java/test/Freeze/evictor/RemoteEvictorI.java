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
    RemoteEvictorI(Ice.ObjectAdapter adapter, String category, Freeze.DB db, StrategyI strategy, Freeze.Evictor evictor)
    {
        _adapter = adapter;
        _category = category;
        _db = db;
        _strategy = strategy;
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
        ServantI servant = new ServantI(this, _evictor, value);
        _evictor.createObject(ident, servant);
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
        _lastSavedValue = -1;
        return result;
    }

    public void
    clearLastSavedValue(Ice.Current current)
    {
        _lastSavedValue = -1;
    }

    public int
    getLastEvictedValue(Ice.Current current)
    {
        return _strategy.getLastEvictedValue();
    }

    public void
    clearLastEvictedValue(Ice.Current current)
    {
        _strategy.clearLastEvictedValue();
    }

    public void
    deactivate(Ice.Current current)
    {
        _evictorAdapter.deactivate();
        _evictorAdapter.waitForDeactivate();
        _adapter.remove(Ice.Util.stringToIdentity(_category));
        _db.close();
    }

    void
    setLastSavedValue(int value)
    {
        _lastSavedValue = value;
    }

    private Ice.ObjectAdapter _adapter;
    private String _category;
    private Freeze.DB _db;
    private StrategyI _strategy;
    private Freeze.Evictor _evictor;
    private Ice.ObjectAdapter _evictorAdapter;
    private int _lastSavedValue;
}
