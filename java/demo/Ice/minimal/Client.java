// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        HelloPrx hello = HelloPrxHelper.checkedCast(communicator.stringToProxy("hello:tcp -p 10000"));
        if(hello == null)
        {
            System.err.println("invalid proxy");
            return 1;
        }

        hello.sayHello();

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
        
        System.exit(status);
    }
}
