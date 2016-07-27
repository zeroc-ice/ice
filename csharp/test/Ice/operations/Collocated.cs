// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Collocated
{
    private static int run(String[] args, Ice.Communicator communicator)
    {
        communicator.getProperties().setProperty("TestAdapter.AdapterId", "test");
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010:udp");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.ObjectPrx prx = adapter.add(new MyDerivedClassI(), Ice.Util.stringToIdentity("test"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.

        if(prx.ice_getConnection() != null)
        {
            throw new Exception();
        }

        AllTests.allTests(communicator);

        return 0;
    }

    public static int Main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);
            initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2"); // For nested AMI.
            initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
            initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");

            //
            // Its possible to have batch oneway requests dispatched
            // after the adapter is deactivated due to thread
            // scheduling so we supress this warning.
            //
            initData.properties.setProperty("Ice.Warn.Dispatch", "0");

            communicator = Ice.Util.initialize(ref args, initData);
            status = run(args, communicator);
        }
        catch(Exception ex)
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

        return status;
    }
}
