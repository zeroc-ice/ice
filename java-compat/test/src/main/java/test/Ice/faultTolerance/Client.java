// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.faultTolerance;

import java.io.PrintWriter;


public class Client extends test.Util.Application
{
    private static void
    usage()
    {
        System.err.println("Usage: Client port...");
    }

    @Override
    public int
    run(String[] args)
    {
        PrintWriter out = getWriter();

        Ice.Communicator communicator = communicator();
        java.util.List<Integer> ports = new java.util.ArrayList<Integer>(args.length);
        for(String arg : args)
        {
            if(arg.charAt(0) == '-')
            {
                //
                // TODO: Arguments recognized by the communicator are not
                // removed from the argument list.
                //
                //System.err.println("Client: unknown option `" + arg + "'");
                //usage();
                //return 1;
                continue;
            }

            int port = 0;
            try
            {
                port = Integer.parseInt(arg);
            }
            catch(NumberFormatException ex)
            {
                ex.printStackTrace();
                return 1;
            }
            ports.add(port);
        }

        if(ports.isEmpty())
        {
                        out.println("Client: no ports specified");
            usage();
            return 1;
        }

        int[] arr = new int[ports.size()];
        for(int i = 0; i < arr.length; i++)
        {
            arr[i] = ports.get(i).intValue();
        }

        try
        {
            AllTests.allTests(this, arr);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            AllTests.test(false);
        }

        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = super.getInitData(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.faultTolerance");
        //
        // This test aborts servers, so we don't want warnings.
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");

        return initData;
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        int result = app.main("Client", args);
        System.gc();
        System.exit(result);
    }
}
