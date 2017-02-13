// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class PluginInitializeFailFactory : Ice.PluginFactory
{
    public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args)
    {
        return new PluginInitializeFail();
    }

    internal class PluginInitializeFail : Ice.Plugin
    {
        public void initialize()
        {
            throw new PluginInitializeFailException();
        }

        public void destroy()
        {
            test(false);
        }

        private static void test(bool b)
        {
            if(!b)
            {
                throw new System.Exception();
            }
        }
    }
}
