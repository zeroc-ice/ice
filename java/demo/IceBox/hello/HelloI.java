// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class HelloI extends Hello
{
    public
    HelloI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public void
    sayHello(Ice.Current current)
    {
        System.out.println("Hello World!");
    }

    public void
    shutdown(Ice.Current current)
    {
        System.out.println("Shutting down...");
        _communicator.shutdown();
    }

    private Ice.Communicator _communicator;
}
