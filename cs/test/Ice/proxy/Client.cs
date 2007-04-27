// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class Client
{
    private static int run(String[] args, Ice.Communicator communicator)
    {
        Test.MyClassPrx myClass = AllTests.allTests(communicator, false);
        
        myClass.shutdown();
        
        return 0;
    }
    
    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;
        
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);
            initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2"); // For nested AMI.
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

            communicator = Ice.Util.initialize(ref args, initData);
            status = run(args, communicator);
        }
        catch(System.Exception ex)
        {
            Console.Error.WriteLine(ex);
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
                Console.Error.WriteLine(ex);
                status = 1;
            }
        }
        
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
