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

public class FreezeService extends Ice.LocalObjectImpl implements IceBox.FreezeService
{
    public void
    start(String name, Ice.Communicator communicator, String[] args, Freeze.DBEnvironment dbEnv)
        throws IceBox.FailureException
    {
	Freeze.DB db = dbEnv.openDB("testdb", true);

	Ice.Properties properties = communicator.getProperties();

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter(name);
        Ice.Object object = new TestI(adapter, communicator.getProperties());
        adapter.add(object, Ice.Util.stringToIdentity(properties.getProperty(name + ".Identity")));
        adapter.activate();
    }

    public void
    stop()
    {
    }
}
