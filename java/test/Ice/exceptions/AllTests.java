// **********************************************************************
//
// Copyright (c) 2002
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

public class AllTests
{
    private static class MyExceptionFactory implements Ice.UserExceptionFactory
    {
        public void
        createAndThrow(String type)
            throws Ice.UserException
        {
            if(type.equals("::A"))
            {
                throw new A();
            }
            else if(type.equals("::B"))
            {
                throw new B();
            }
            else if(type.equals("::C"))
            {
                throw new C();
            }
            else if(type.equals("::D"))
            {
                throw new D();
            }
            assert(false); // Should never be reached
        }

        public void
        destroy()
        {
            // Nothing to do
        }
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static ThrowerPrx
    allTests(Ice.Communicator communicator, boolean collocated)
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

	{
	    System.out.print("testing servant locator registration exceptions... ");
	    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter2");
	    Ice.ServantLocator loc = new ServantLocatorI();
	    adapter.addServantLocator(loc, "x");
	    boolean gotException = false;
	    try
            {
		adapter.addServantLocator(loc, "x");
	    }
	    catch(Ice.AlreadyRegisteredException ex)
	    {
		gotException = true;
	    }
	    test(gotException);

	    gotException = false;
	    adapter.removeServantLocator("x");
	    try
            {
		adapter.removeServantLocator("x");
	    }
	    catch(Ice.NotRegisteredException ex)
	    {
		gotException = true;
	    }
	    test(gotException);
	    adapter.deactivate();
	    System.out.println("ok");
	}

	{
	    System.out.print("testing object factory registration exceptions... ");
	    Ice.ObjectFactory of = new ObjectFactoryI();
	    communicator.addObjectFactory(of, "::x");
	    boolean gotException = false;
	    try
            {
		communicator.addObjectFactory(of, "::x");
	    }
	    catch(Ice.AlreadyRegisteredException ex)
	    {
		gotException = true;
	    }
	    test(gotException);

	    gotException = false;
	    communicator.removeObjectFactory("::x");
	    try
            {
		communicator.removeObjectFactory("::x");
	    }
	    catch(Ice.NotRegisteredException ex)
	    {
		gotException = true;
	    }
	    test(gotException);
	    System.out.println("ok");
	}

	{
	    System.out.print("testing user exception factory registration exceptions... ");
	    Ice.UserExceptionFactory f = new MyExceptionFactory();
	    communicator.addUserExceptionFactory(f, "::x");
	    boolean gotException = false;
	    try
            {
		communicator.addUserExceptionFactory(f, "::x");
	    }
	    catch(Ice.AlreadyRegisteredException ex)
	    {
		gotException = true;
	    }
	    test(gotException);

	    gotException = false;
	    communicator.removeUserExceptionFactory("::x");
	    try
            {
		communicator.removeUserExceptionFactory("::x");
	    }
	    catch(Ice.NotRegisteredException ex)
	    {
		gotException = true;
	    }
	    test(gotException);
	    System.out.println("ok");
	}

        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "thrower:default -p 12345 -t 2000";
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

        if(!collocated) // If the server is collocated, exception factories are not needed.
        {
            //
            // NOTE: Factories will be dynamically installed in Java
            //

            System.out.print("catching derived types w/ dynamic exception factories... ");
            System.out.flush();

            try
            {
                thrower.throwBasA(1, 2);
                test(false);
            }
            catch(Ice.NoUserExceptionFactoryException ex)
            {
                test(false);
            }
            catch(A ex)
            {
                test(ex instanceof B);
            }

            try
            {
                thrower.throwCasA(1, 2, 3);
                test(false);
            }
            catch(Ice.NoUserExceptionFactoryException ex)
            {
            }
            catch(A ex)
            {
                test(ex instanceof C);
            }

            try
            {
                thrower.throwCasB(1, 2, 3);
                test(false);
            }
            catch(Ice.NoUserExceptionFactoryException ex)
            {
                test(false);
            }
            catch(B ex)
            {
                test(ex instanceof C);
            }

            System.out.println("ok");

            System.out.print("catching derived types w/ static exception factories... ");
            System.out.flush();

            //
            // Remove dynamically-installed factories.
            //
            communicator.removeUserExceptionFactory("::B");
            communicator.removeUserExceptionFactory("::C");

            Ice.UserExceptionFactory factory = new MyExceptionFactory();
            communicator.addUserExceptionFactory(factory, "::A");
            communicator.addUserExceptionFactory(factory, "::B");
            communicator.addUserExceptionFactory(factory, "::C");
            communicator.addUserExceptionFactory(factory, "::D");
        }
        else
        {
            System.out.print("catching derived types... ");
            System.out.flush();
        }

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
	    test(!collocated);

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
	
	System.out.print("catching object not exist exception... ");
	System.out.flush();
	
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
		test(ex.facet.length == 1);
		test(ex.facet[0].equals("no such facet"));
	    }

	    ThrowerPrx thrower3 = ThrowerPrxHelper.uncheckedCast(thrower2, "no such facet either");
	    try
	    {
		thrower3.ice_ping();
		test(false);
	    }
	    catch(Ice.FacetNotExistException ex)
	    {
		test(ex.facet.length == 2);
		test(ex.facet[0].equals("no such facet"));
		test(ex.facet[1].equals("no such facet either"));
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
        catch(Ice.TimeoutException ex)
        {
	    //
	    // We get the original exception with collocation
	    // optimization.
	    //
	    test(collocated);
	}
        catch(Ice.UnknownLocalException ex)
        {
	    //
	    // We get the an unknown local exception without
	    // collocation optimization.
	    //
	    test(!collocated);
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
	    //
	    // We get the an unknown exception without collocation
	    // optimization.
	    //
	    test(!collocated);
        }
        catch(RuntimeException ex)
        {
	    //
	    // We get the original exception with collocation
	    // optimization.
	    //
	    test(collocated);
        }

        System.out.println("ok");

        return thrower;
    }
}
