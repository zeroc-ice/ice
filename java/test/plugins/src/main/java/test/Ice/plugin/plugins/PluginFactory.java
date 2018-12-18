// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.plugin.plugins;

public class PluginFactory implements com.zeroc.Ice.PluginFactory
{
    @Override
    public com.zeroc.Ice.Plugin create(com.zeroc.Ice.Communicator communicator, String name, String[] args)
    {
        return new Plugin(args);
    }

    static class Plugin implements com.zeroc.Ice.Plugin
    {
        public Plugin(String[] args)
        {
            _args = args;
        }

        @Override
        public void initialize()
        {
            _initialized = true;
            test(_args.length == 3);
            test(_args[0].equals("C:\\Program Files\\"));
            test(_args[1].equals("--DatabasePath"));
            test(_args[2].equals("C:\\Program Files\\Application\\db"));
        }

        @Override
        public void destroy()
        {
            _destroyed = true;
        }

        @SuppressWarnings("deprecation")
        @Override
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

        private String[] _args;
        private boolean _initialized = false;
        private boolean _destroyed = false;
    }
}
