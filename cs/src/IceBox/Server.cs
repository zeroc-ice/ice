// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class Server : Ice.Application
{
    private static void usage()
    {
        Console.Error.WriteLine("Usage: IceBox.Server [options] --Ice.Config=<file>\n");
        Console.Error.WriteLine(
            "Options:\n" +
            "-h, --help           Show this message.\n"
        );
    }

    public override int run(string[] args)
    {
	for(int i = 1; i < args.Length; ++i)
	{
	    if(args[i].Equals("-h") || args[i].Equals("--help"))
	    {
		usage();
		return 0;
	    }
	    else if(!args[i].StartsWith("--"))
	    {
		Console.Error.WriteLine("Server: unknown option `" + args[i] + "'");
		usage();
		return 1;
	    }
	}

	ServiceManagerI serviceManagerImpl = new ServiceManagerI(args);
	return serviceManagerImpl.run();
    }

    public static void Main(string[] args)
    {
	Server server = new Server();
	int status = server.main(args);
        System.Environment.Exit(status);
    }
}
