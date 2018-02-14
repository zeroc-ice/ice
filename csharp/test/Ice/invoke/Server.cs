// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

public class Server
{
    public static int run(string[] args, Ice.Communicator communicator)
    {
        bool async = false;
        for(int i = 0; i < args.Length; ++i)
        {
            if(args[i].Equals("--async"))
            {
               async = true;
            }
        }

        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010:udp");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.addServantLocator(new ServantLocatorI(async), "");
        adapter.activate();

        communicator.waitForShutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);
            communicator = Ice.Util.initialize(ref args, initData);
            status = run(args, communicator);
        }
        catch(System.Exception ex)
        {
            Console.Error.WriteLine(ex);
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        return status;
    }

}
