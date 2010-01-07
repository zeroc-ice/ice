// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{
    public static void Main(string[] args)
    {
        try
        {
            Ice.Communicator communicator = Ice.Util.initialize(ref args);
            Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("Hello", "tcp -p 10000");
            adapter.add(new HelloI(), communicator.stringToIdentity("hello"));
            adapter.activate();
            communicator.waitForShutdown();
            communicator.destroy();
        }
        catch(System.Exception ex)
        {
            System.Console.Error.WriteLine(ex);
            System.Environment.Exit(1);
        }
    }
}
