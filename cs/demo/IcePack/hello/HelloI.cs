// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using Demo;

public class HelloI : Hello
{
    public HelloI(string n)
    {
        name = n;
    }

    public override void sayHello(Ice.Current current)
    {
        Console.WriteLine(name + " says Hello World!");
    }

    public override void destroy(Ice.Current current)
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
        catch(IcePack.ObjectNotExistException)
        {
            Debug.Assert(false);
        }

        //
        // Remove the Hello object from the object adapter.
        //
        adapter.remove(current.id);

        string id = communicator.getProperties().getProperty("Identity");

        Console.WriteLine("HelloFactory-" + id + ": destroyed Hello object named '" + name + "'");
    }
}
