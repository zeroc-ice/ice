// Copyright (c) ZeroC, Inc.

public class PluginTwoFactory : Ice.PluginFactory
{
    public string pluginName => "Test";

    public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args) => new PluginTwo(communicator);

    internal class PluginTwo : BasePlugin
    {
        public PluginTwo(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void initialize()
        {
            _other = (BasePlugin)_communicator.getPluginManager().getPlugin("PluginOne");
            test(_other.isInitialized());
            _initialized = true;
        }

        public override void destroy()
        {
            _destroyed = true;
            test(!_other.isDestroyed());
        }
    }
}
