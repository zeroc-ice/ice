// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


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

    public static Test.MyClassPrx
    allTests(Ice.Communicator communicator, boolean collocated)
    {
        System.out.print("testing stringToProxy... ");
        System.out.flush();
        String ref = "test:default -p 12345 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        System.out.println("ok");

        System.out.print("testing checked cast... ");
        System.out.flush();
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(base);
        test(cl != null);
        Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrxHelper.checkedCast(cl);
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

	if(!collocated)
	{
	    System.out.print("testing twoway operations with AMI... ");
	    System.out.flush();
	    TwowaysAMI.twowaysAMI(cl);
	    TwowaysAMI.twowaysAMI(derived);
	    System.out.println("ok");
	}

        return cl;
    }
}
