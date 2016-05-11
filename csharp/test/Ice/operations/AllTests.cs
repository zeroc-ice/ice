// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class AllTests : TestCommon.TestApp
{
    public static Test.MyClassPrx allTests(Ice.Communicator communicator)
    {
        Flush();
        string rf = "test:default -p 12010";
        Ice.ObjectPrx baseProxy = communicator.stringToProxy(rf);
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);
        Test.MyDerivedClassPrx derivedProxy = Test.MyDerivedClassPrxHelper.checkedCast(cl);

        Write("testing twoway operations... ");
        Flush();
        Twoways.twoways(communicator, cl);
        Twoways.twoways(communicator, derivedProxy);
        derivedProxy.opDerived();
        WriteLine("ok");

        Write("testing oneway operations... ");
        Flush();
        Oneways.oneways(communicator, cl);
        WriteLine("ok");

        Write("testing twoway operations with AMI... ");
        Flush();
        TwowaysAMI.twowaysAMI(communicator, cl);
        TwowaysAMI.twowaysAMI(communicator, derivedProxy);
        WriteLine("ok");

        Write("testing oneway operations with AMI... ");
        Flush();
        OnewaysAMI.onewaysAMI(communicator, cl);
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
