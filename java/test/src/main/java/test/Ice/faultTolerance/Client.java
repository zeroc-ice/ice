// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.faultTolerance;

import java.io.PrintWriter;

public class Client extends test.TestHelper
{
    private static void
    usage()
    {
        System.err.println("Usage: Client port...");
    }

    public void run(String[] args)
    {
        java.util.List<String> remainingArgs = new java.util.ArrayList<String>();
        com.zeroc.Ice.Properties properties = createTestProperties(args, remainingArgs);
        properties.setProperty("Ice.Package.Test", "test.Ice.faultTolerance");
        //
        // This test aborts servers, so we don't want warnings.
        //
        properties.setProperty("Ice.Warn.Connections", "0");

        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            PrintWriter out = getWriter();
            java.util.List<Integer> ports = new java.util.ArrayList<>(args.length);
            for(String arg : remainingArgs)
            {
                if(arg.charAt(0) == '-')
                {
                    usage();
                    throw new IllegalArgumentException("Client: unknown option `" + arg + "'");
                }
                ports.add(Integer.parseInt(arg));
            }

            if(ports.isEmpty())
            {
                usage();
                throw new RuntimeException("Client: no ports specified");
            }

            int[] arr = new int[ports.size()];
            for(int i = 0; i < arr.length; i++)
            {
                arr[i] = ports.get(i).intValue();
            }
            AllTests.allTests(this, arr);
        }
    }
}
