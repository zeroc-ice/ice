// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
            test(ex.a == 1);
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
            test(ex.a == 1);
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
            test(ex.d == -1);
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
            test(ex.a == 1);
            test(ex.b == 2);
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
            test(ex.a == 1);
            test(ex.b == 2);
            test(ex.c == 3);
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
            test(ex.a == 1);
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
            test(ex.a == 1);
            test(ex.b == 2);
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
                assert(ex instanceof B);
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
                assert(ex instanceof C);
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
                assert(ex instanceof C);
            }

            System.out.println("ok");

            System.out.print("catching derived types w/ static exception factories... ");
            System.out.flush();

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
            test(ex.a == 1);
            test(ex.b == 2);
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
            test(ex.a == 1);
            test(ex.b == 2);
            test(ex.c == 3);
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
            test(ex.a == 1);
            test(ex.b == 2);
            test(ex.c == 3);
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
	    thrower2.ice_ping();
	    test(false);
        }
        catch(Ice.FacetNotExistException ex)
        {
	    test(ex.facet.equals("no such facet"));
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
        catch(Exception ex)
        {
            test(false);
        }

        System.out.println("ok");

        return thrower;
    }
}
