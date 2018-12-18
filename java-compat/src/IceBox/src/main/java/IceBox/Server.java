// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
            "-h, --help           Show this message.\n"
        );
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

            final String prefix = "IceBox.Service.";
            Ice.Properties properties = communicator.getProperties();
            java.util.Map<String, String> services = properties.getPropertiesForPrefix(prefix);

            for(String arg : argHolder.value)
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

            ServiceManagerI serviceManagerImpl = new ServiceManagerI(communicator, argHolder.value);
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
