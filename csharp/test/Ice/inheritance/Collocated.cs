// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

public class Collocated : TestCommon.Application
{
    public override int run(string[] args)
    {
        communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.Object obj = new InitialI(adapter);
        adapter.add(obj, Ice.Util.stringToIdentity("initial"));
        AllTests.allTests(this);
        return 0;
    }

    public static int Main(string[] args)
    {
        Collocated app = new Collocated();
        return app.runmain(args);
    }
}
