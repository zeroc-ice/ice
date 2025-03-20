// Copyright (c) ZeroC, Inc.

public class PluginTwoFailFactory : Ice.PluginFactory
{
    public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args) => new PluginTwoFail(communicator);

    internal class PluginTwoFail : BasePluginFail
    {
        public PluginTwoFail(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void initialize()
        {
            _one = (BasePluginFail)_communicator.getPluginManager().getPlugin("PluginOneFail");
            test(_one.isInitialized());
            _three = (BasePluginFail)_communicator.getPluginManager().getPlugin("PluginThreeFail");
            test(!_three.isInitialized());
            _initialized = true;
        }

        public override void destroy()
        {
            test(!_one.isDestroyed());
            //
            // Not destroyed because initialize fails.
            //
            test(!_three.isDestroyed());
            _destroyed = true;
        }
    }
}
