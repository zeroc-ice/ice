// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestCommon.Application
{
    public static Ice.Value MyValueFactory(string type)
    {
        if(type.Equals("::Test::I"))
        {
            return new II();
        }
        else if(type.Equals("::Test::J"))
        {
            return new JI();
        }
        else if(type.Equals("::Test::H"))
        {
            return new HI();
        }
        Debug.Assert(false); // Should never be reached
        return null;
    }

    public override int run(string[] args)
    {
        communicator().getValueFactoryManager().add(MyValueFactory, "::Test::I");
        communicator().getValueFactoryManager().add(MyValueFactory, "::Test::J");
        communicator().getValueFactoryManager().add(MyValueFactory, "::Test::H");

        communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.Object @object = new InitialI(adapter);
        adapter.add(@object, Ice.Util.stringToIdentity("initial"));
        @object = new UnexpectedObjectExceptionTestI();
        adapter.add(@object, Ice.Util.stringToIdentity("uoet"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        Server app = new Server();
        return app.runmain(args);
    }
}
