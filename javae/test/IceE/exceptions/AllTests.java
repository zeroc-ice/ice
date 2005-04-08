// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class Callback
    {
	Callback()
	{
	    _called = false;
	}

	public synchronized boolean
	check()
	{
	    while(!_called)
	    {
		try
		{
		    wait(5000);
		}
		catch(InterruptedException ex)
		{
		    continue;
		}

		if(!_called)
		{
		    return false; // Must be timeout.
		}
	    }

	    _called = false;
	    return true;
	}
	
	public synchronized void
	called()
	{
	    assert(!_called);
	    _called = true;
	    notify();
	}

	private boolean _called;
    }

    public static ThrowerPrx
    allTests(Ice.Communicator communicator)
    {
        {
	    System.out.print("testing servant registration exceptions... ");
	    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter1");
	    Ice.Object obj = new EmptyI();
	    adapter.add(obj, Ice.Util.stringToIdentity("x"));
	    boolean gotException = false;
	    try
            {
		adapter.add(obj, Ice.Util.stringToIdentity("x"));
	    }
	    catch(Ice.AlreadyRegisteredException ex)
	    {
		gotException = true;
	    }
	    test(gotException);

	    gotException = false;
	    adapter.remove(Ice.Util.stringToIdentity("x"));
	    try
            {
		adapter.remove(Ice.Util.stringToIdentity("x"));
	    }
	    catch(Ice.NotRegisteredException ex)
	    {
		gotException = true;
	    }
	    test(gotException);
	    adapter.deactivate();
	    System.out.println("ok");
	}

        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "thrower:default -p 12345 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        ThrowerPrx thrower = ThrowerPrxHelper.checkedCast(base);
        test(thrower != null);
        test(thrower.equals(base));
        System.out.println("ok");

        System.out.print("catching exact types... ");
        System.out.flush();

        try
        {
            thrower.throwAasA(1);
            test(false);
        }
        catch(A ex)
        {
	    System.out.println(ex.aMem);
            test(ex.aMem == 1);
        }
        catch(Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwAorDasAorD(1);
            test(false);
        }
        catch(A ex)
        {
            test(ex.aMem == 1);
        }
        catch(Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwAorDasAorD(-1);
            test(false);
        }
        catch(D ex)
        {
            test(ex.dMem == -1);
        }
        catch(Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwBasB(1, 2);
            test(false);
        }
        catch(B ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        catch(Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwCasC(1, 2, 3);
            test(false);
        }
        catch(C ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        catch(Exception ex)
        {
            test(false);
        }

        System.out.println("ok");

        System.out.print("catching base types... ");
        System.out.flush();

        try
        {
            thrower.throwBasB(1, 2);
            test(false);
        }
        catch(A ex)
        {
            test(ex.aMem == 1);
        }
        catch(Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwCasC(1, 2, 3);
            test(false);
        }
        catch(B ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        catch(Exception ex)
        {
            test(false);
        }

        System.out.println("ok");

	System.out.print("catching derived types... ");
	System.out.flush();

        try
        {
            thrower.throwBasA(1, 2);
            test(false);
        }
        catch(B ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        catch(Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwCasA(1, 2, 3);
            test(false);
        }
        catch(C ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        catch(Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwCasB(1, 2, 3);
            test(false);
        }
        catch(C ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        catch(Exception ex)
        {
            test(false);
        }

        System.out.println("ok");

	if(thrower.supportsUndeclaredExceptions())
	{
	    System.out.print("catching unknown user exception... ");
	    System.out.flush();
	    
	    try
	    {
		thrower.throwUndeclaredA(1);
		test(false);
	    }
	    catch(Ice.UnknownUserException ex)
	    {
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    
	    try
	    {
		thrower.throwUndeclaredB(1, 2);
		test(false);
	    }
	    catch(Ice.UnknownUserException ex)
	    {
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    
	    try
	    {
		thrower.throwUndeclaredC(1, 2, 3);
		test(false);
	    }
	    catch(Ice.UnknownUserException ex)
	    {
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    
	    System.out.println("ok");
	}
	
	if(thrower.supportsAssertException())
	{
	    System.out.print("testing assert in the server... ");
	    System.out.flush();
	    
	    try
	    {
		thrower.throwAssertException();
		test(false);
	    }
	    catch(Ice.ConnectionLostException ex)
	    {
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	    
	    System.out.println("ok");
	}

	System.out.print("catching object not exist exception... ");
	System.out.flush();

	{
	    Ice.Identity id = Ice.Util.stringToIdentity("does not exist");
	    try
	    {
		ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower.ice_newIdentity(id));
		thrower2.ice_ping();
		test(false);
	    }
	    catch(Ice.ObjectNotExistException ex)
	    {
		test(ex.id.equals(id));
	    }
	    catch(Exception ex)
	    {
		test(false);
	    }
	}

        System.out.println("ok");

        System.out.print("catching facet not exist exception... ");
        System.out.flush();
 
	try
	{
	    ThrowerPrx thrower2 = ThrowerPrxHelper.uncheckedCast(thrower, "no such facet");
	    try
	    {
		thrower2.ice_ping();
		test(false);
	    }
	    catch(Ice.FacetNotExistException ex)
	    {
		test(ex.facet.equals("no such facet"));
	    }
	}
        catch(Exception ex)
        {
            test(false);
        }

        System.out.println("ok");

        System.out.print("catching operation not exist exception... ");
        System.out.flush();

        try
        {
	    WrongOperationPrx thrower2 = WrongOperationPrxHelper.uncheckedCast(thrower);
	    thrower2.noSuchOperation();
	    test(false);
        }
        catch(Ice.OperationNotExistException ex)
        {
	    test(ex.operation.equals("noSuchOperation"));
        }
        catch(Exception ex)
        {
            test(false);
        }

        System.out.println("ok");

        System.out.print("catching unknown local exception... ");
        System.out.flush();

        try
        {
            thrower.throwLocalException();
            test(false);
        }
        catch(Ice.UnknownLocalException ex)
        {
	}
        catch(Exception ex)
        {
            test(false);
        }

        System.out.println("ok");

        System.out.print("catching unknown non-Ice exception... ");
        System.out.flush();

        try
        {
            thrower.throwNonIceException();
            test(false);
        }
        catch(Ice.UnknownException ex)
        {
        }

        System.out.println("ok");

        return thrower;
    }
}
