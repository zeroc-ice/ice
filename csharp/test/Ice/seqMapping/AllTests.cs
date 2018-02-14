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
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        initData.properties.setProperty("Ice.FactoryAssemblies", "seqMapping,version=1.0.0.0");
        return initData;
    }

    override
    public void run(Ice.Communicator communicator)
#else
    public static Test.MyClassPrx allTests(Ice.Communicator communicator, bool collocated)
#endif
    {
#if SILVERLIGHT
        bool collocated = false;
#endif
        Flush();
        string rf = "test:default -p 12010";
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
#if SILVERLIGHT
        cl.shutdown();
#else
        return cl;
#endif
    }
}
