// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;

public class Admin : Ice.Application
{
    private static void usage()
    {
        Console.Error.WriteLine(
            "Usage: " + appName() + " [options] [command...]\n" +
            "Options:\n" +
            "-h, --help          Show this message.\n" +
            "\n" +
            "Commands:\n" +
            "start SERVICE       Start a service." +
            "stop SERVICE        Stop a service." +
            "shutdown            Shutdown the server.");
    }

    public override int run(string[] args)
    {
        ArrayList commands = new ArrayList();

	for(int idx = 0; idx < args.Length; ++idx)
        {
            if(args[idx].Equals("-h") || args[idx].Equals("--help"))
            {
                usage();
                return 0;
            }
            else if(args[idx].StartsWith("-"))
            {
                Console.Error.WriteLine(appName() + ": unknown option `" + args[idx] + "'");
                usage();
                return 1;
            }
            else
            {
                commands.Add(args[idx]);
            }
        }

        if(commands.Count == 0)
        {
            usage();
            return 1;
        }

        Ice.Properties properties = communicator().getProperties();

	Ice.Identity managerIdentity = new Ice.Identity();
	managerIdentity.category = properties.getPropertyWithDefault("IceBox.InstanceName", "IceBox");
	managerIdentity.name = "ServiceManager";

	string managerProxy;
	if(properties.getProperty("Ice.Default.Locator").Length == 0)
	{
	    string managerEndpoints = properties.getProperty("IceBox.ServiceManager.Endpoints");
	    if(managerEndpoints.Length == 0)
	    {
	        Console.Error.WriteLine(appName() + ": property `IceBox.ServiceManager.Endpoints' is not set");
	        return 1;
	    }

	    managerProxy = "\"" + communicator().identityToString(managerIdentity) + "\" :" + managerEndpoints;
	}
	else
	{
	    string managerAdapterId = properties.getProperty("IceBox.ServiceManager.AdapterId");
	    if(managerAdapterId.Length == 0)
	    {
	        Console.Error.WriteLine(appName() + ": property `IceBox.ServiceManager.AdapterId' is not set");
	        return 1;
	    }

	    managerProxy = "\"" + communicator().identityToString(managerIdentity) + "\" @" + managerAdapterId;
	}

        Ice.ObjectPrx obj = communicator().stringToProxy(managerProxy);
        IceBox.ServiceManagerPrx manager = IceBox.ServiceManagerPrxHelper.checkedCast(obj);
        if(manager == null)
        {
            Console.Error.WriteLine(appName() + ": `" + managerProxy + "' is not running");
            return 1;
        }

	for(int i = 0; i < commands.Count; ++i)
	{
	    string command = (string)commands[i];
            if(command.Equals("shutdown"))
            {
                manager.shutdown();
            }
            else if(command.Equals("start"))
	    {
	        if(++i >= commands.Count)
		{
                    Console.Error.WriteLine(appName() + ": no service name specified.");
		    return 1;
		}

		string service = (string)commands[i];
		try
		{
		    manager.startService(service);
		}
		catch(IceBox.NoSuchServiceException)
		{
                    Console.Error.WriteLine(appName() + ": unknown service `" + service + "'");
		}
		catch(IceBox.AlreadyStartedException)
		{
                    Console.Error.WriteLine(appName() + ": service already started.");
		}
	    }
            else if(command.Equals("stop"))
	    {
	        if(++i >= commands.Count)
		{
                    Console.Error.WriteLine(appName() + ": no service name specified.");
		    return 1;
		}

		string service = (string)commands[i];
		try
		{
		    manager.stopService(service);
		}
		catch(IceBox.NoSuchServiceException)
		{
                    Console.Error.WriteLine(appName() + ": unknown service `" + service + "'");
		}
		catch(IceBox.AlreadyStoppedException)
		{
                    Console.Error.WriteLine(appName() + ": service already stopped.");
		}
	    }
            else
            {
                Console.Error.WriteLine(appName() + ": unknown command `" + command + "'");
                usage();
                return 1;
            }
        }

        return 0;
    }

    public static void Main(string[] args)
    {
        Admin admin = new Admin();
	int status = admin.main(args);
	if(status != 0)
	{
	    System.Environment.Exit(status);
	}
    }
}
