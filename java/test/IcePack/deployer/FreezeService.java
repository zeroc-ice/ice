// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class FreezeService extends Ice.LocalObjectImpl implements IceBox.FreezeService
{
    public void
    start(String name, Ice.Communicator communicator, String[] args, Freeze.DBEnvironment dbEnv)
        throws IceBox.FailureException
    {
	Freeze.DB db = dbEnv.openDB("testdb", true);

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter(name);
        Ice.Object object = new TestI(adapter, communicator.getProperties());
        adapter.add(object, Ice.Util.stringToIdentity(name));
        adapter.activate();
    }

    public void
    stop()
    {
    }
}
