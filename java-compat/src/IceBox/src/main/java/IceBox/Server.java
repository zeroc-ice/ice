//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceBox;

public final class Server
{
    static class ShutdownHook extends Thread
    {
        private Ice.Communicator _communicator;
        private final java.lang.Object _doneMutex = new java.lang.Object();
        private boolean _done = false;

        ShutdownHook(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        @Override
        public void
        run()
        {
            _communicator.shutdown();

            synchronized(_doneMutex)
            {
                //
                // Wait on the server to finish shutting down before exiting the ShutdownHook. This ensures
                // that all IceBox services have had a chance to shutdown cleanly before the JVM terminates.
                //
                while(!_done)
                {
                    try
                    {
                        _doneMutex.wait();
                    }
                    catch(InterruptedException ex)
                    {
                        break;
                    }
                }
            }
        }

        public void
        done()
        {
            synchronized(_doneMutex)
            {
                _done = true;
                _doneMutex.notify();
            }
        }
    }

    private static void
    usage()
    {
        System.err.println("Usage: IceBox.Server [options] --Ice.Config=<file>\n");
        System.err.println(
            "Options:\n" +
            "-h, --help           Show this message.\n" +
            "-v, --version        Display the Ice version."
        );
    }

    private static int
    run(Ice.Communicator communicator, String[] argSeq)
    {
        final String prefix = "IceBox.Service.";
        Ice.Properties properties = communicator.getProperties();
        java.util.Map<String, String> services = properties.getPropertiesForPrefix(prefix);

        java.util.List<String> iceBoxArgs = new java.util.ArrayList<String>();
        for(String s : argSeq)
        {
            iceBoxArgs.add(s);
        }

        for(String key : services.keySet())
        {
            String name = key.substring(prefix.length());
            for(int i = 0; i < iceBoxArgs.size(); ++i)
            {
                if(iceBoxArgs.get(i).startsWith("--" + name))
                {
                    iceBoxArgs.remove(i);
                    i--;
                }
            }
        }

        for(String arg : iceBoxArgs)
        {
            if(arg.equals("-h") || arg.equals("--help"))
            {
                usage();
                return 0;
            }
            else if(arg.equals("-v") || arg.equals("--version"))
            {
                System.out.println(Ice.Util.stringVersion());
                return 0;
            }
            else
            {
                System.err.println("IceBox.Server: unknown option `" + arg + "'");
                usage();
                return 1;
            }
        }

        ServiceManagerI serviceManagerImpl = new ServiceManagerI(communicator, argSeq);
        return serviceManagerImpl.run();
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.StringSeqHolder argHolder = new Ice.StringSeqHolder(args);

        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        ShutdownHook shutdownHook = null;

        try(Ice.Communicator communicator = Ice.Util.initialize(argHolder, initData))
        {
            shutdownHook = new ShutdownHook(communicator);
            Runtime.getRuntime().addShutdownHook(shutdownHook);

            status = run(communicator, argHolder.value);
        }
        finally
        {
            if(shutdownHook != null)
            {
                shutdownHook.done();
            }
        }

        System.exit(status);
    }
}
