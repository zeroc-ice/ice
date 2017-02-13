// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceDiscovery.simple;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        int num;
        try
        {
            num = args.length == 1 ? Integer.parseInt(args[0]) : 0;
        }
        catch(NumberFormatException ex)
        {
            num = 0;
        }
        AllTests.allTests(this, num);
        return 0;
    }

    public static void main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);

        System.gc();
        System.exit(status);
    }
}
