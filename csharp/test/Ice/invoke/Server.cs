// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

public class ServantLocatorI : Ice.ServantLocator
{
    public ServantLocatorI(bool async)
    {
        if(async)
        {
            _blobject = new BlobjectAsyncI();
        }
        else
        {
            _blobject = new BlobjectI();
        }
    }

    public Ice.Object
    locate(Ice.Current current, out System.Object cookie)
    {
        cookie = null;
        return _blobject;
    }

    public void
    finished(Ice.Current current, Ice.Object servant, System.Object cookie)
    {
    }

    public void
    deactivate(string category)
    {
    }

    private Ice.Object _blobject;
}

public class Server : TestCommon.Application
{
    public override int run(string[] args)
    {
        bool async = false;
        for(int i = 0; i < args.Length; ++i)
        {
            if(args[i].Equals("--async"))
            {
               async = true;
            }
        }

        communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        adapter.addServantLocator(new ServantLocatorI(async), "");
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
