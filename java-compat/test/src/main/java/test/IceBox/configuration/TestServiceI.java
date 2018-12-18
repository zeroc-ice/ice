// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.IceBox.configuration;

public class TestServiceI implements IceBox.Service
{
    @Override
    public void
    start(String name, Ice.Communicator communicator, String[] args)
    {
        communicator.getProperties().setProperty("Ice.Package.Test", "test.IceBox.configuration");

        Ice.ObjectAdapter adapter = communicator.createObjectAdapter(name + "OA");
        adapter.add(new TestI(args), Ice.Util.stringToIdentity("test"));
        adapter.activate();
    }

    @Override
    public void
    stop()
    {
    }
}
