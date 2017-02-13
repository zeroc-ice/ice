// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
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
        var initial = new InitialI(adapter);
        adapter.add(initial, Ice.Util.stringToIdentity("initial"));
        UnexpectedObjectExceptionTestI uet = new UnexpectedObjectExceptionTestI();
        adapter.add(uet, Ice.Util.stringToIdentity("uoet"));
        AllTests.allTests(this);
        // We must call shutdown even in the collocated case for cyclic dependency cleanup
        initial.shutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        Collocated app = new Collocated();
        return app.runmain(args);
    }
}
