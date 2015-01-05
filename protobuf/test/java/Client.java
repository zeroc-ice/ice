// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Protobuf is licensed to you under the terms
// described in the ICE_PROTOBUF_LICENSE file included in this
// distribution.
//
// **********************************************************************

public class Client
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Test.MyClassPrx myClass = AllTests.allTests(communicator, false);
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
            communicator = Ice.Util.initialize(args);
            status = run(args, communicator);
        }
        catch(Exception ex)
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
