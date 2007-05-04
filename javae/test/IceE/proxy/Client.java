// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Client
{
    public static int
    run(String[] args, Ice.Communicator communicator, Ice.InitializationData data, java.io.PrintStream out)
    {
        Test.MyClassPrx myClass = AllTests.allTests(communicator, out);
        myClass.shutdown();
        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            //
            // In this test, we need at least two threads in the
            // client side thread pool for nested AMI.
            //
            Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(argsH);
            initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
            initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");

            //
            // We must set MessageSizeMax to an explicit values,
            // because we run tests to check whether
            // Ice.MemoryLimitException is raised as expected.
            //
            initData.properties.setProperty("Ice.MessageSizeMax", "100");

            //
            // We don't want connection warnings because of the timeout test.
            //
            initData.properties.setProperty("Ice.Warn.Connections", "0");

            //
            // Use a faster connection monitor timeout to test AMI
            // timeouts.
            //
            initData.properties.setProperty("Ice.MonitorConnections", "1");
            
            communicator = Ice.Util.initialize(argsH, initData);
            status = run(argsH.value, communicator, initData, System.out);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.gc();
        System.exit(status);
    }
}
