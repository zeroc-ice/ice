// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class AllTests : TestCommon.AllTests
{
    public static Test.MyClassPrx allTests(TestCommon.Application app)
    {
        Ice.Communicator communicator = app.communicator();
        Flush();
        string rf = "test:" + app.getTestEndpoint(0);
        Ice.ObjectPrx baseProxy = communicator.stringToProxy(rf);
        Test.MyClassPrx cl = Test.MyClassPrxHelper.checkedCast(baseProxy);
        Test.MyDerivedClassPrx derivedProxy = Test.MyDerivedClassPrxHelper.checkedCast(cl);

        Write("testing twoway operations... ");
        Flush();
        Twoways.twoways(app, cl);
        Twoways.twoways(app, derivedProxy);
        derivedProxy.opDerived();
        WriteLine("ok");

        Write("testing oneway operations... ");
        Flush();
        Oneways.oneways(app, cl);
        WriteLine("ok");

        Write("testing twoway operations with AMI... ");
        Flush();
        TwowaysAMI.twowaysAMI(app, cl);
        TwowaysAMI.twowaysAMI(app, derivedProxy);
        WriteLine("ok");

        Write("testing oneway operations with AMI... ");
        Flush();
        OnewaysAMI.onewaysAMI(app, cl);
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
