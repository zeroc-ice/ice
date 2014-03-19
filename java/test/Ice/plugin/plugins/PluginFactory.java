// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.plugin.plugins;

public class PluginFactory implements Ice.PluginFactory
{
    public Ice.Plugin create(Ice.Communicator communicator, String name, String[] args)
    {
        return new Plugin(communicator, args);
    }

    static class Plugin implements Ice.Plugin
    {
        public Plugin(Ice.Communicator communicator, String[] args)
        {
            _communicator = communicator;
            _args = args;
        }

        public void initialize()
        {
            _initialized = true;
            test(_args.length == 3);
            test(_args[0].equals("C:\\Program Files\\"));
            test(_args[1].equals("--DatabasePath"));
            test(_args[2].equals("C:\\Program Files\\Application\\db"));
        }

        public void destroy()
        {
            _destroyed = true;
        }

        protected void finalize() throws Throwable
        {
            try
            {
                if(!_initialized)
                {
                    System.out.println("test.Ice.plugin.plugins.Plugin not initialized");
                }
                if(!_destroyed)
                {
                    System.out.println("test.Ice.plugin.plugins.Plugin not destroyed");
                }
            }
            finally
            {
                super.finalize();
            }
        }

        private static void test(boolean b)
        {
            if(!b)
            {
                throw new RuntimeException();
            }
        }

        private Ice.Communicator _communicator;
        private String[] _args;
        private boolean _initialized = false;
        private boolean _destroyed = false;
    }
}
