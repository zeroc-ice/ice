// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class HelloFactoryI extends _HelloFactoryDisp
{
    public HelloPrx
    create(String name, Ice.Current current)
        throws NameExistsException
    {
        Ice.ObjectAdapter adapter = current.adapter;
        Ice.Communicator communicator = adapter.getCommunicator();

        //
        // Create the servant and add it to the object adapter using the
        // given name as the identity.
        //
        Ice.Object hello = new HelloI(name);
        Ice.ObjectPrx object = adapter.add(hello, Ice.Util.stringToIdentity(name));

        //
        // Get the IcePack Admin interface and register the newly created
        // Hello object with the IcePack object registry.
        // 
        try
        {
            IcePack.AdminPrx admin = IcePack.AdminPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Admin"));
            admin.addObject(object);
        }
        catch(IcePack.ObjectExistsException ex)
        {
            //
            // An object with the same identity is already registered with
            // the registry. Remove the object from the object adapter and
            // throw.
            //
            adapter.remove(object.ice_getIdentity());
            throw new NameExistsException();
        }
	catch(IcePack.DeploymentException ex)
	{
	    assert(false);
	}

        String id = communicator.getProperties().getProperty("Identity");

        System.out.println("HelloFactory-" + id + ": created Hello object named '" + name + "'");

        return HelloPrxHelper.uncheckedCast(object);
    }

    public HelloPrx
    find(String name, Ice.Current current)
        throws NameNotExistException
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
        catch(Ice.NoEndpointException ex)
        {
            //
            // The object couldn't be activated. Ignore.
            // 
            return HelloPrxHelper.uncheckedCast(communicator.stringToProxy(name));
        }
        catch(Ice.NotRegisteredException ex)
        {
            //
            // The object is not registered.
            //
            throw new NameNotExistException();
        }
        catch(Ice.ObjectNotExistException ex)
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
            catch(IcePack.ObjectNotExistException e)
            {
                assert(false);
            }
            throw new NameNotExistException();
        }
    }
}
