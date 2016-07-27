// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server
{
    internal class App : Ice.Application
    {
        public override int run(string[] args)
        {
            communicator().getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010");
            communicator().getProperties().setProperty("Ice.Warn.Dispatch", "0");

            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            adapter.addServantLocator(new ServantLocatorI("category"), "category");
            adapter.addServantLocator(new ServantLocatorI(""), "");
            adapter.add(new TestI(), Ice.Util.stringToIdentity("asm"));
            adapter.add(new TestActivationI(), Ice.Util.stringToIdentity("test/activation"));
            adapter.activate();
            adapter.waitForDeactivate();
            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args);
    }
}
