// Copyright (c) ZeroC, Inc.

public class PluginInitializeFailFactory : Ice.PluginFactory
{
    public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args) => new PluginInitializeFail();

    internal class PluginInitializeFail : Ice.Plugin
    {
        public void initialize() => throw new PluginInitializeFailException();

        public void destroy() => global::Test.TestHelper.test(false);
    }
}
