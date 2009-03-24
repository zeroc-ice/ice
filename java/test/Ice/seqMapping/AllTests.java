// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
        Ice.ObjectPrx baseProxy = communicator.stringToProxy(ref);
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);

        System.out.print("testing twoway operations... ");
        System.out.flush();
        Twoways.twoways(communicator, cl);
        System.out.println("ok");

        if(!collocated)
        {
            System.out.print("testing twoway operations with AMI... ");
            System.out.flush();
            TwowaysAMI.twowaysAMI(communicator, cl);
            System.out.println("ok");
        }

        return cl;
    }
}
