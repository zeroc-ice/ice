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
            admin.removeObject(adapter.createProxy(current.id));
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
