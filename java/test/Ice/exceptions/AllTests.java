// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
            if (type.equals("::A"))
            {
                throw new A();
            }
            else if (type.equals("::B"))
            {
                throw new B();
            }
            else if (type.equals("::C"))
            {
                throw new C();
            }
            else if (type.equals("::D"))
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
        if (!b)
        {
            throw new RuntimeException();
        }
    }

    public static ThrowerPrx
    allTests(Ice.Communicator communicator, boolean collocated)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref;

        Ice.Properties properties = communicator.getProperties();

        String protocol = properties.getProperty("Ice.Protocol");
        String secure = "";

        if (protocol == null)
        {
            protocol = "tcp";
        }

        if (protocol.equals("ssl"))
        {
            secure = " -s ";
        }

        String endpts = protocol + " -p 12345 -t 2000";

        ref = "thrower" + secure + ":" + endpts;

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
        catch (A ex)
        {
            test(ex.a == 1);
        }
        catch (Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwAorDasAorD(1);
            test(false);
        }
        catch (A ex)
        {
            test(ex.a == 1);
        }
        catch (Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwAorDasAorD(-1);
            test(false);
        }
        catch (D ex)
        {
            test(ex.d == -1);
        }
        catch (Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwBasB(1, 2);
            test(false);
        }
        catch (B ex)
        {
            test(ex.a == 1);
            test(ex.b == 2);
        }
        catch (Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwCasC(1, 2, 3);
            test(false);
        }
        catch (C ex)
        {
            test(ex.a == 1);
            test(ex.b == 2);
            test(ex.c == 3);
        }
        catch (Exception ex)
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
        catch (A ex)
        {
            test(ex.a == 1);
        }
        catch (Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwCasC(1, 2, 3);
            test(false);
        }
        catch (B ex)
        {
            test(ex.a == 1);
            test(ex.b == 2);
        }
        catch (Exception ex)
        {
            test(false);
        }

        System.out.println("ok");

        if (!collocated) // If the server is collocated, exception factories
        {                // are not needed.
            System.out.print("catching derived types w/o exception " +
                              "factories... ");
            System.out.flush();

            try
            {
                thrower.throwBasA(1, 2);
                test(false);
            }
            catch (Ice.NoUserExceptionFactoryException ex)
            {
            }
            catch (Exception ex)
            {
                test(false);
            }

            try
            {
                thrower.throwCasA(1, 2, 3);
                test(false);
            }
            catch (Ice.NoUserExceptionFactoryException ex)
            {
            }
            catch (Exception ex)
            {
                test(false);
            }

            try
            {
                thrower.throwCasB(1, 2, 3);
                test(false);
            }
            catch (Ice.NoUserExceptionFactoryException ex)
            {
            }
            catch (Exception ex)
            {
                test(false);
            }

            System.out.println("ok");

            System.out.print("catching derived types w/ exception " +
                              "factories... ");
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
        catch (B ex)
        {
            test(ex.a == 1);
            test(ex.b == 2);
        }
        catch (Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwCasA(1, 2, 3);
            test(false);
        }
        catch (C ex)
        {
            test(ex.a == 1);
            test(ex.b == 2);
            test(ex.c == 3);
        }
        catch (Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwCasB(1, 2, 3);
            test(false);
        }
        catch (C ex)
        {
            test(ex.a == 1);
            test(ex.b == 2);
            test(ex.c == 3);
        }
        catch (Exception ex)
        {
            test(false);
        }

        System.out.println("ok");

        System.out.print("catching unknown user exception... ");
        System.out.flush();

        try
        {
            thrower.throwUndeclaredA(1);
            test(false);
        }
        catch (Ice.UnknownUserException ex)
        {
        }
        catch (Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwUndeclaredB(1, 2);
            test(false);
        }
        catch (Ice.UnknownUserException ex)
        {
        }
        catch (Exception ex)
        {
            test(false);
        }

        try
        {
            thrower.throwUndeclaredC(1, 2, 3);
            test(false);
        }
        catch (Ice.UnknownUserException ex)
        {
        }
        catch (Exception ex)
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
        catch (Ice.UnknownLocalException ex)
        {
        }
        catch (Exception ex)
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
        catch (Ice.UnknownException ex)
        {
        }
        catch (Exception ex)
        {
            test(false);
        }

        System.out.println("ok");

        return thrower;
    }
}
