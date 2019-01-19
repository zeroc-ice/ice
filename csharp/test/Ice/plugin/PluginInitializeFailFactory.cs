//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
