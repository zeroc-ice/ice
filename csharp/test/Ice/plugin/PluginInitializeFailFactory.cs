// Copyright (c) ZeroC, Inc.

namespace Ice.plugin;

public class PluginInitializeFailFactory : Ice.PluginFactory
{
    public string pluginName => "Test";

    public Plugin create(Communicator communicator, string name, string[] args) => new PluginInitializeFail();

    internal class PluginInitializeFail : Plugin
    {
        public void initialize() => throw new PluginInitializeFailException();

        public void destroy() => Test.TestHelper.test(false);
    }
}
