// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{
    public static int Main(string[] args)
    {
        try
        {
            using(Ice.Communicator communicator = Ice.Util.initialize(ref args))
            {
                Ice.ObjectAdapter adapter =
                    communicator.createObjectAdapterWithEndpoints("Hello", "default -h localhost -p 10000");
                adapter.add(new HelloI(), communicator.stringToIdentity("hello"));
                adapter.activate();
                communicator.waitForShutdown();
            }
            return 0;
        }
        catch(System.Exception ex)
        {
            System.Console.Error.WriteLine(ex);
            return 1;
        }
    }
}
