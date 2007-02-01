// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
