// Copyright (c) ZeroC, Inc.

public class PluginOneFactory : Ice.PluginFactory
{
    public string pluginName => "Test";

    public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args) => new PluginOne(communicator);

    internal class PluginOne : BasePlugin
    {
        public PluginOne(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void initialize()
        {
            _other = (BasePlugin)_communicator.getPluginManager().getPlugin("PluginTwo");
            test(!_other.isInitialized());
            _initialized = true;
        }

        public override void destroy()
        {
            _destroyed = true;
            test(_other.isDestroyed());
        }
    }
}
