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
    RemoteEvictorFactoryI(Ice.ObjectAdapter adapter, String envName)
    {
        _adapter = adapter;
	_envName = envName;
    }

    static class Initializer extends Ice.LocalObjectImpl implements Freeze.ServantInitializer
    {
        public void
        initialize(Ice.ObjectAdapter adapter, Ice.Identity ident, Ice.Object servant)
        {
	    ServantI servantImpl =  (ServantI) ((Test._ServantTie) servant).ice_delegate();
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
    createEvictor(String name, Ice.Current current)
    {
        Freeze.Evictor evictor = Freeze.Util.createEvictor(_adapter.getCommunicator(), _envName, name, null, true);

        RemoteEvictorI remoteEvictor = new RemoteEvictorI(_adapter, name, evictor);
        evictor.installServantInitializer(new Initializer(remoteEvictor, evictor));
        return Test.RemoteEvictorPrxHelper.uncheckedCast(_adapter.add(remoteEvictor, Ice.Util.stringToIdentity(name)));
    }

    public void
    shutdown(Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
    }

    private Ice.ObjectAdapter _adapter;
    private String _envName;
}
