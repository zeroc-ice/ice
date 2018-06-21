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
    public static Test.MyClassPrx allTests(Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        Flush();
        string rf = "test:" + helper.getTestEndpoint(0);
        Ice.ObjectPrx baseProxy = communicator.stringToProxy(rf);
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);
        Test.MyDerivedClassPrx derivedProxy = Test.MyDerivedClassPrxHelper.checkedCast(cl);

        Write("testing twoway operations... ");
        Flush();
        Twoways.twoways(helper, cl);
        Twoways.twoways(helper, derivedProxy);
        derivedProxy.opDerived();
        WriteLine("ok");

        Write("testing oneway operations... ");
        Flush();
        Oneways.oneways(helper, cl);
        WriteLine("ok");

        Write("testing twoway operations with AMI... ");
        Flush();
        TwowaysAMI.twowaysAMI(helper, cl);
        TwowaysAMI.twowaysAMI(helper, derivedProxy);
        WriteLine("ok");

        Write("testing oneway operations with AMI... ");
        Flush();
        OnewaysAMI.onewaysAMI(helper, cl);
        WriteLine("ok");

        Write("testing batch oneway operations... ");
        Flush();
        BatchOneways.batchOneways(cl);
        BatchOneways.batchOneways(derivedProxy);
        WriteLine("ok");

        Write("testing batch AMI oneway operations... ");
        Flush();
        BatchOnewaysAMI.batchOneways(cl);
        BatchOnewaysAMI.batchOneways(derivedProxy);
        WriteLine("ok");
        return cl;
    }
}
