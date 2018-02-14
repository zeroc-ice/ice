// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

#if SILVERLIGHT
using System.Windows.Controls;
#endif

public class AllTests : TestCommon.TestApp
{
#if SILVERLIGHT
    public override Ice.InitializationData initData()
    {
        //
        // In this test, we need at least two threads in the
        // client side thread pool for nested AMI.
        //
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");

	initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");
        return initData;
    }

    override
    public void run(Ice.Communicator communicator)
#else
    public static Test.MyClassPrx allTests(Ice.Communicator communicator)
#endif
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
#if SILVERLIGHT
        cl.shutdown();
#else
        return cl;
#endif
    }
}
