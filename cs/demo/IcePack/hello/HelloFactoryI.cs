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

public class HelloFactoryI : _HelloFactoryDisp
{
    public override HelloPrx create(string name, Ice.Current current)
    {
        Ice.ObjectAdapter adapter = current.adapter;
        Ice.Communicator communicator = adapter.getCommunicator();

        //
        // Create the servant and add it to the object adapter using the
        // given name as the identity.
        //
        Ice.Object hello = new HelloI(name);
        Ice.ObjectPrx @object = adapter.add(hello, Ice.Util.stringToIdentity(name));

        //
        // Get the IcePack Admin interface and register the newly created
        // Hello object with the IcePack object registry.
        // 
        try
        {
            IcePack.AdminPrx admin = IcePack.AdminPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Admin"));
            admin.addObject(@object);
        }
        catch(IcePack.ObjectExistsException)
        {
            //
            // An object with the same identity is already registered with
            // the registry. Remove the object from the object adapter and
            // throw.
            //
            adapter.remove(@object.ice_getIdentity());
            throw new NameExistsException();
        }
	catch(IcePack.DeploymentException)
	{
	    Debug.Assert(false);
	}

        String id = communicator.getProperties().getProperty("Identity");

        Console.WriteLine("HelloFactory-" + id + ": created Hello object named '" + name + "'");

        return HelloPrxHelper.uncheckedCast(@object);
    }

    public override HelloPrx find(string name, Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();

        //
        // The object is registered with the IcePack object registry so we
        // just return a proxy containing the identity.
        //
        try
        {
            return HelloPrxHelper.checkedCast(communicator.stringToProxy(name));
        }
        catch(Ice.NoEndpointException)
        {
            //
            // The object couldn't be activated. Ignore.
            // 
            return HelloPrxHelper.uncheckedCast(communicator.stringToProxy(name));
        }
        catch(Ice.NotRegisteredException)
        {
            //
            // The object is not registered.
            //
            throw new NameNotExistException();
        }
        catch(Ice.ObjectNotExistException)
        {
            //
            // The object doesn't exist anymore. This can occur if the
            // server has been restarted and the server objects haven't
            // been removed from the object registry.
            // 
            IcePack.AdminPrx admin = IcePack.AdminPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Admin"));
            try
            {
                admin.removeObject(communicator.stringToProxy(name));	
            }
            catch(IcePack.ObjectNotExistException)
            {
                Debug.Assert(false);
            }
            throw new NameNotExistException();
        }
    }
}
