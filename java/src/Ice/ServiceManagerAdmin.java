// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public class ServiceManagerAdmin
{
    private static void
    usage()
    {
        System.err.println(
            "usage: Ice.ServiceManagerAdmin [options] [command]\n" +
            "\n" +
            "Options:\n" +
            "\n" +
            "  -h          Display this message.\n" +
            "\n" +
            "Commands:\n" +
            "\n" +
            "  shutdown    Shutdown the server.");
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        if (args.length == 0)
        {
            usage();
            return 1;
        }

        int i = 0;
        while (i < args.length && args[i].charAt(0) == '-')
        {
            if (args[i].equals("-h"))
            {
                usage();
                return 0;
            }
            else
            {
                System.err.println("unknown option `" + args[i] + "'");
                usage();
                return 1;
            }
            //
            // Unreachable until there are more options
            //
            //i++;
        }

        Ice.Properties properties = communicator.getProperties();
        final String refProperty = "ServiceManager";
        String ref = properties.getProperty(refProperty);
        if (ref.length() == 0)
        {
            System.err.println("property `" + refProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        Ice.ServiceManagerPrx mgr =
            Ice.ServiceManagerPrxHelper.checkedCast(base.ice_twoway().ice_timeout(-1).ice_secure(false));
        if (mgr == null)
        {
            System.err.println("invalid object reference");
            return 1;
        }

        while (i < args.length)
        {
            if (args[i].equals("shutdown"))
            {
                mgr.shutdown();
            }
            else
            {
                System.err.println("unknown command `" + args[i] + "'");
                usage();
                return 1;
            }
            i++;
        }

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            StringSeqHolder argsH = new StringSeqHolder(args);
            communicator = Util.initialize(argsH);
            status = run(argsH.value, communicator);
        }
        catch (Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if (communicator != null)
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
