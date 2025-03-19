// Copyright (c) ZeroC, Inc.

public class PluginThreeFailFactory : Ice.PluginFactory
{
    public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args)
    {
        return new PluginThreeFail(communicator);
    }

    internal class PluginThreeFail : BasePluginFail
    {
        public PluginThreeFail(Ice.Communicator communicator) : base(communicator)
        {
        }

        public override void initialize()
        {
            throw new PluginInitializeFailException();
        }

        public override void destroy()
        {
            test(false);
        }
    }
}
