// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    createEvictor(String name, Test.EvictorPersistenceMode mode, Ice.Current current)
    {
        Freeze.DB db = _dbEnv.openDB(name, true);

        Freeze.EvictorPersistenceMode fMode;
        if(mode == Test.EvictorPersistenceMode.SaveUponEviction)
        {
            fMode = Freeze.EvictorPersistenceMode.SaveUponEviction;
        }
        else
        {
            fMode = Freeze.EvictorPersistenceMode.SaveAfterMutatingOperation;
        }
        Freeze.Evictor evictor = db.createEvictor(fMode);
        _adapter.addServantLocator(evictor, name);

        RemoteEvictorI remoteEvictor = new RemoteEvictorI(_adapter, name, db, evictor);
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
