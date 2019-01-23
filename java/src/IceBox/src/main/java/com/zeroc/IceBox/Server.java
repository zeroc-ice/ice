//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceBox;

public final class Server
{
    static class ShutdownHook extends Thread
    {
        private com.zeroc.Ice.Communicator _communicator;
        private final java.lang.Object _doneMutex = new java.lang.Object();
        private boolean _done = false;

        ShutdownHook(com.zeroc.Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        @Override
        public void run()
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

        public void done()
        {
            synchronized(_doneMutex)
            {
                _done = true;
                _doneMutex.notify();
            }
        }
    }

    private static void usage()
    {
        System.err.println("Usage: com.zeroc.IceBox.Server [options] --Ice.Config=<file>\n");
        System.err.println(
            "Options:\n" +
            "-h, --help           Show this message.\n" +
            "-v, --version        Display the Ice version."
        );
    }

    private static int run(com.zeroc.Ice.Communicator communicator, java.util.List<String> argSeq)
    {
        final String prefix = "IceBox.Service.";
        com.zeroc.Ice.Properties properties = communicator.getProperties();
        java.util.Map<String, String> services = properties.getPropertiesForPrefix(prefix);

        java.util.List<String> iceBoxArgs = new java.util.ArrayList<String>(argSeq);

        for(String key : services.keySet())
        {
            String name = key.substring(prefix.length());
            iceBoxArgs.removeIf(v -> v.startsWith("--" + name));
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
                System.out.println(com.zeroc.Ice.Util.stringVersion());
                return 0;
            }
            else
            {
                System.err.println("IceBox.Server: unknown option `" + arg + "'");
                usage();
                return 1;
            }
        }

        ServiceManagerI serviceManagerImpl = new ServiceManagerI(communicator, argSeq.toArray(new String[0]));
        return serviceManagerImpl.run();
    }

    public static void main(String[] args)
    {
        int status = 0;
        java.util.List<String> argSeq = new java.util.ArrayList<String>();

        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.properties = com.zeroc.Ice.Util.createProperties();
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        ShutdownHook shutdownHook = null;

        try(com.zeroc.Ice.Communicator communicator = com.zeroc.Ice.Util.initialize(args, initData, argSeq))
        {
            shutdownHook = new ShutdownHook(communicator);
            Runtime.getRuntime().addShutdownHook(shutdownHook);

            status = run(communicator, argSeq);
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
