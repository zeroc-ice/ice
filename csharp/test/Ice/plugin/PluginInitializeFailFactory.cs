//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using ZeroC.Ice;

namespace ZeroC.Ice.Test.Plugin
{
    public class PluginInitializeFailFactory : IPluginFactory
    {
        public IPlugin Create(Communicator communicator, string name, string[] args) => new PluginInitializeFail();

        internal class PluginInitializeFail : IPlugin
        {
            public void Initialize() => throw new PluginInitializeFailException();

            public void Destroy() => TestHelper.Assert(false);
        }
    }
}
