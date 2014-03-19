// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Server
{
    public static void
    main(String[] args)
    {
        try
        {
            Ice.Communicator communicator = Ice.Util.initialize(args);
            Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("Hello", "tcp -h localhost -p 10000");
            adapter.add(new HelloI(), communicator.stringToIdentity("hello"));
            adapter.activate();
            communicator.waitForShutdown();
            communicator.destroy();
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            System.exit(1);
        }
    }
}
