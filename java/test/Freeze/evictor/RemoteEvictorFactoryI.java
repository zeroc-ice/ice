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

public final class RemoteEvictorFactoryI extends Test._RemoteEvictorFactoryDisp
{
    RemoteEvictorFactoryI(Ice.ObjectAdapter adapter, Freeze.DBEnvironment dbEnv)
    {
        _adapter = adapter;
        _dbEnv = dbEnv;
    }

    static class Initializer extends Ice.LocalObjectImpl implements Freeze.ServantInitializer
    {
        public void
        initialize(Ice.ObjectAdapter adapter, Ice.Identity ident, Ice.Object servant)
        {
            ServantI servantImpl = (ServantI)servant;
            servantImpl.init(_remoteEvictor, _evictor);
        }

        Initializer(RemoteEvictorI remoteEvictor, Freeze.Evictor evictor)
        {
            _remoteEvictor = remoteEvictor;
            _evictor = evictor;
        }

        private RemoteEvictorI _remoteEvictor;
        private Freeze.Evictor _evictor;
    }

    public Test.RemoteEvictorPrx
    createEvictor(String name, Test.Strategy s, Ice.Current current)
    {
        Freeze.DB db = _dbEnv.openDB(name, true);

        Freeze.PersistenceStrategy delegate;
        if(s == Test.Strategy.Eviction)
        {
            delegate = db.createEvictionStrategy();
        }
        else
        {
            delegate = db.createIdleStrategy();
        }
        StrategyI strategy = new StrategyI(delegate);
        Freeze.Evictor evictor = db.createEvictor(strategy);

        RemoteEvictorI remoteEvictor = new RemoteEvictorI(_adapter, name, db, strategy, evictor);
        evictor.installServantInitializer(new Initializer(remoteEvictor, evictor));
        return Test.RemoteEvictorPrxHelper.uncheckedCast(_adapter.add(remoteEvictor, Ice.Util.stringToIdentity(name)));
    }

    public void
    shutdown(Ice.Current current)
    {
        _dbEnv.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
    private Freeze.DBEnvironment _dbEnv;
}
