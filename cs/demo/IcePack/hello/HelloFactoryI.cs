// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using Demo;

public class HelloFactoryI : HelloFactoryDisp_
{
    public override HelloPrx create(string name, Ice.Current current)
    {
        Ice.ObjectAdapter adapter = current.adapter;
        Ice.Communicator communicator = adapter.getCommunicator();

        //
        // Create the servant and add it to the object adapter using the
        // given name as the identity.
        //
        Ice.ObjectPrx @object;
	try
	{
	    @object = adapter.add(new HelloI(name), Ice.Util.stringToIdentity(name));
	}
	catch(Ice.AlreadyRegisteredException)
	{
	    //
	    // The object is already registered.
	    //
	    throw new NameExistsException();
	}

        //
        // Get the IcePack Admin interface and register the newly created
        // Hello object with the IcePack object registry.
        // 
	IcePack.AdminPrx admin = IcePack.AdminPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Admin"));
	try
        {
            admin.addObject(@object);
        }
        catch(IcePack.ObjectExistsException)
        {
	    //
	    // This should only occur if the server has been restarted and
	    // the server objects haven't been removed from the object
	    // registry.
	    //
	    // In this case remove the current object, and re-add.
	    // 
            try
            {
		admin.removeObject(@object.ice_getIdentity());
		admin.addObject(@object);
            }
            catch(IcePack.ObjectNotExistException)
            {
                Debug.Assert(false);
            }
	    catch(IcePack.ObjectExistsException)
	    {
		Debug.Assert(false);
	    }
	    catch(IcePack.DeploymentException)
	    {
		Debug.Assert(false);
	    }
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
                admin.removeObject(Ice.Util.stringToIdentity(name));
            }
            catch(IcePack.ObjectNotExistException)
            {
                Debug.Assert(false);
            }
            throw new NameNotExistException();
        }
    }
}
