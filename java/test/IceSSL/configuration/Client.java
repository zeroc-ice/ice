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
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        if(args.length < 1)
        {
            System.out.println("Usage: client testdir");
            return 1;
        }

        Test.ServerFactoryPrx factory;

        //
        // If we're using the JDK 1.4 plugin, we can only use thread-per-connection.
        // Otherwise, we run the test twice, once for each concurrency model.
        //
        System.out.println("testing with thread-per-connection.");
        factory = AllTests.allTests(communicator, args[0], false);

        if(!IceSSL.Util.jdkTarget.equals("1.4"))
        {
            System.out.println("testing with thread pool.");
            AllTests.allTests(communicator, args[0], true);
        }

        factory.shutdown();

        return 0;
    }

    public static void main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
            communicator = Ice.Util.initialize(argsH);
            status = run(argsH.value, communicator);
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
