// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package IceBox;

public final class Server extends Ice.Application
{
    private static void
    usage()
    {
        System.err.println("Usage: IceBox.Server [options] --Ice.Config=<file>\n");
        System.err.println(
            "Options:\n" +
            "-h, --help           Show this message.\n"
        );
    }

    public static void
    main(String[] args)
    {
	Server server = new Server();
	server.main("IceBox.Server", args);
    }

    public int
    run(String[] args)
    {
	for(int i = 1; i < args.length; ++i)
	{
	    if(args[i].equals("-h") || args[i].equals("--help"))
	    {
		usage();
		return 0;
	    }
	    else if(!args[i].startsWith("--"))
	    {
		System.err.println("Server: unknown option `" + args[i] + "'");
		usage();
		return 1;
	    }
	}

	ServiceManagerI serviceManagerImpl = new ServiceManagerI(this, args);
	return serviceManagerImpl.run();
    }
}
