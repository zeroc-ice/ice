// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
using System.Diagnostics;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    internal class App : Ice.Application
    {
        public override int run(string[] args)
        {
            TestIntfPrx obj = AllTests.allTests(communicator());
            obj.shutdown();
            return 0;
        }
    }

    public static int Main(string[] args)
    {
        Ice.InitializationData data = new Ice.InitializationData();
#if COMPACT
        //
        // When using Ice for .NET Compact Framework, we need to specify
        // the assembly so that Ice can locate classes and exceptions.
        //
        data.properties = Ice.Util.createProperties();
        data.properties.setProperty("Ice.FactoryAssemblies", "client");
#endif

        App app = new App();
        return app.main(args, data);
    }
}
