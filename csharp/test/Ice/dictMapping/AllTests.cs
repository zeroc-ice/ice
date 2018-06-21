// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class AllTests : Test.AllTests
{
    public static Test.MyClassPrx allTests(Test.TestHelper helper, bool collocated)
    {
        Ice.Communicator communicator = helper.communicator();
        Flush();
        string rf = "test:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx baseProxy = communicator.stringToProxy(rf);
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);

        Write("testing twoway operations... ");
        Flush();
        Twoways.twoways(communicator, cl);
        WriteLine("ok");

        if(!collocated)
        {
            Write("testing twoway operations with AMI... ");
            Flush();
            TwowaysAMI.twowaysAMI(communicator, cl);
            WriteLine("ok");
        }
        return cl;
    }
}
