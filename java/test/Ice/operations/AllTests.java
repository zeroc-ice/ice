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
    private static void
    test(boolean b)
    {
        if (!b)
        {
            throw new RuntimeException();
        }
    }

    public static Test.MyClassPrx
    allTests(Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();

        String address = properties.getProperty("Ice.Address");
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

        String ref = "test" + secure + ":" + protocol + " -p 12345 -t 2000";
        if (address != null)
        {
            ref += " -h " + address;
        }

        System.out.print("testing stringToProxy... ");
        System.out.flush();
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(base);
        test(cl != null);
        Test.MyDerivedClassPrx derived =
            Test.MyDerivedClassPrxHelper.checkedCast(cl);
        test(derived != null);
        test(cl.equals(base));
        test(derived.equals(base));
        test(cl.equals(derived));
        System.out.println("ok");

        System.out.print("testing twoway operations... ");
        System.out.flush();
        Twoways.twoways(cl);
        Twoways.twoways(derived);
        derived.opDerived();
        System.out.println("ok");

        return cl;
    }
}
