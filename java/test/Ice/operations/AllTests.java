// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class AllTests
{
    public static Test.MyClassPrx
    allTests(Ice.Communicator communicator, boolean collocated)
    {
        String ref = "test:default -p 12010 -t 10000";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(base);
        Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrxHelper.checkedCast(cl);

        System.out.print("testing twoway operations... ");
        System.out.flush();
        Twoways.twoways(communicator, cl);
        Twoways.twoways(communicator, derived);
        derived.opDerived();
        System.out.println("ok");

        if(!collocated)
        {
            System.out.print("testing twoway operations with AMI... ");
            System.out.flush();
            TwowaysAMI.twowaysAMI(communicator, cl);
            TwowaysAMI.twowaysAMI(communicator, derived);
            System.out.println("ok");

            System.out.print("testing batch oneway operations... ");
            System.out.flush();
            BatchOneways.batchOneways(cl);
            BatchOneways.batchOneways(derived);
            System.out.println("ok");
        }

        return cl;
    }
}
