// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
