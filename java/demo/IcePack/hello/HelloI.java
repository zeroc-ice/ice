// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class HelloI extends Hello
{
    HelloI(String n)
    {
        name = n;
    }

    public void
    sayHello(Ice.Current current)
    {
        System.out.println(name + " says Hello World!");
    }

    public void
    destroy(Ice.Current current)
    {
        Ice.ObjectAdapter adapter = current.adapter;
        Ice.Communicator communicator = adapter.getCommunicator();

        //
        // Get the IcePack Admin interface and remove the Hello object
        // from the IcePack object registry.
        // 
        IcePack.AdminPrx admin = IcePack.AdminPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Admin"));
        try
        {
            admin.removeObject(current.id);
        }
        catch(IcePack.ObjectNotExistException ex)
        {
            assert(false);
        }

        //
        // Remove the Hello object from the object adapter.
        //
        adapter.remove(current.id);

        String id = communicator.getProperties().getProperty("Identity");

        System.out.println("HelloFactory-" + id + ": destroyed Hello object named '" + name + "'");
    }
}
