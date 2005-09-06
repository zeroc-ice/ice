// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
	    IceUtil.Debug.Assert(!_called);
	    _called = true;
	    notify();
	}

	private boolean _called;
    }

    public static ThrowerPrx
    allTests(Ice.Communicator communicator, java.io.PrintStream out)
    {
	{
	    out.print("testing object adapter registration exceptions... ");
	    Ice.ObjectAdapter first = communicator.createObjectAdapter("TestAdapter0");
	    try
	    {
		Ice.ObjectAdapter second = communicator.createObjectAdapter("TestAdapter0");
		test(false);
	    }
	    catch(Ice.AlreadyRegisteredException ex)
	    {
		// Expected
	    }

	    out.println("ok");
	}
	
        {
	    out.print("testing servant registration exceptions... ");
	    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter1");
	    Ice.Object obj = new EmptyI();
	    adapter.add(obj, Ice.Util.stringToIdentity("x"));
	    try
            {
		adapter.add(obj, Ice.Util.stringToIdentity("x"));
		test(false);
	    }
	    catch(Ice.AlreadyRegisteredException ex)
	    {
	    }

	    adapter.remove(Ice.Util.stringToIdentity("x"));
	    try
            {
		adapter.remove(Ice.Util.stringToIdentity("x"));
		test(false);
	    }
	    catch(Ice.NotRegisteredException ex)
	    {
	    }
	    adapter.deactivate();
	    out.println("ok");
	}

        out.print("testing stringToProxy... ");
        out.flush();
        String ref = communicator.getProperties().getPropertyWithDefault("Test.Proxy", 
		"thrower:default -p 12345 -t 10000");
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        ThrowerPrx thrower = ThrowerPrxHelper.checkedCast(base);
        test(thrower != null);
        test(thrower.equals(base));
        out.println("ok");

        out.print("catching exact types... ");
        out.flush();

        try
        {
            thrower.throwAasA(1);
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

        out.println("ok");

        out.print("catching base types... ");
        out.flush();

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

        out.println("ok");

	out.print("catching derived types... ");
	out.flush();

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

        out.println("ok");

	if(thrower.supportsUndeclaredExceptions())
	{
	    out.print("catching unknown user exception... ");
	    out.flush();
	    
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
	    
	    out.println("ok");
	}
	
	if(thrower.supportsAssertException())
	{
	    out.print("testing assert in the server... ");
	    out.flush();
	    
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
	    
	    out.println("ok");
	}

	out.print("catching object not exist exception... ");
	out.flush();

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

        out.println("ok");

        out.print("catching facet not exist exception... ");
        out.flush();
 
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

        out.println("ok");

        out.print("catching operation not exist exception... ");
        out.flush();

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

        out.println("ok");

        out.print("catching unknown local exception... ");
        out.flush();

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

        out.println("ok");

        out.print("catching unknown non-Ice exception... ");
        out.flush();

        try
        {
            thrower.throwNonIceException();
            test(false);
        }
        catch(Ice.UnknownException ex)
        {
        }

        out.println("ok");

        return thrower;
    }
}
