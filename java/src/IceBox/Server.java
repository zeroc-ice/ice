// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceBox;

public final class Server extends Ice.Application
{
    private static void
    usage()
    {
        System.err.println("Usage: IceBox.Server [options]\n");
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
	Ice.Properties properties = communicator().getProperties();
	Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
	argsH.value = properties.parseCommandLineOptions("IceBox", argsH.value);
	
	for(int i = 1; i < argsH.value.length; ++i)
	{
	    if(argsH.value[i].equals("-h") || argsH.value[i].equals("--help"))
	    {
		usage();
		return 0;
	    }
	    else if(!argsH.value[i].startsWith("--"))
	    {
		System.err.println("Server: unknown option `" + argsH.value[i] + "'");
		usage();
		return 1;
	    }
	}

	ServiceManagerI serviceManagerImpl = new ServiceManagerI(this, argsH.value);
	return serviceManagerImpl.run();
    }
}
