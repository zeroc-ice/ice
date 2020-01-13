//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class PluginInitializeFailFactory : Ice.IPluginFactory
{
    public Ice.IPlugin create(Ice.Communicator communicator, string name, string[] args) => new PluginInitializeFail();

    internal class PluginInitializeFail : Ice.IPlugin
    {
        public void initialize() => throw new PluginInitializeFailException();

        public void destroy() => test(false);

        private static void test(bool b)
        {
            if (!b)
            {
                throw new System.Exception();
            }
        }
    }
}
