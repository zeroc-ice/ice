// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Protobuf is licensed to you under the terms
// described in the ICE_PROTOBUF_LICENSE file included in this
// distribution.
//
// **********************************************************************

import Demo.*;

public class HelloI extends _HelloDisp
{
    public void
    sayHello(tutorial.PersonPB.Person p, Ice.Current current)
    {
        System.out.println("Hello World from " + p);
    }

    public void
    shutdown(Ice.Current current)
    {
        System.out.println("Shutting down...");
        current.adapter.getCommunicator().shutdown();
    }
}
