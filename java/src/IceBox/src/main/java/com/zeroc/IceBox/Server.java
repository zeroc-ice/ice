// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

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
            "-h, --help           Show this message.\n"
        );
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

            final String prefix = "IceBox.Service.";
            com.zeroc.Ice.Properties properties = communicator.getProperties();
            java.util.Map<String, String> services = properties.getPropertiesForPrefix(prefix);

            for(String arg : argSeq)
            {
                boolean valid = false;
                for(java.util.Map.Entry<String, String> entry : services.entrySet())
                {
                    String name = entry.getKey().substring(prefix.length());
                    if(arg.startsWith("--" + name))
                    {
                        valid = true;
                        break;
                    }
                }
                if(!valid)
                {
                    if(arg.equals("-h") || arg.equals("--help"))
                    {
                        usage();
                        status = 1;
                        break;
                    }
                    else
                    {
                        System.err.println("IceBox.Server: unknown option `" + arg + "'");
                        usage();
                        status = 1;
                        break;
                    }
                }
            }

            ServiceManagerI serviceManagerImpl = new ServiceManagerI(communicator, args);
            status = serviceManagerImpl.run();
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
