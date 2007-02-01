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
        AllTests.allTests(communicator);
        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(args);
            initData.properties.setProperty("Ice.Default.Locator", "locator:default -p 12010");
            communicator = Ice.Util.initialize(args, initData);
            status = run(args, communicator);
        }
        catch (Ice.LocalException ex)
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
            catch (Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.gc();
        System.exit(status);
    }
}
