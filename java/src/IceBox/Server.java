// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceBox;

public final class Server
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
        Ice.Communicator communicator = null;
        int status = 0;

        try
        {
            Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
            communicator = Ice.Util.initialize(argsH);

            Ice.Properties properties = communicator.getProperties();
            argsH.value = properties.parseCommandLineOptions("IceBox", argsH.value);

            for(int i = 1; i < argsH.value.length; ++i)
            {
                if(argsH.value[i].equals("-h") || argsH.value[i].equals("--help"))
                {
                    usage();
                    System.exit(0);
                }
                else if(!argsH.value[i].startsWith("--"))
                {
                    System.err.println("Server: unknown option `" + argsH.value[i] + "'");
                    usage();
                    System.exit(1);
                }
            }

            ServiceManagerI serviceManagerImpl = new ServiceManagerI(communicator, argsH.value);
            status = serviceManagerImpl.run();
        }
        catch (Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch (Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.exit(status);
    }
}
