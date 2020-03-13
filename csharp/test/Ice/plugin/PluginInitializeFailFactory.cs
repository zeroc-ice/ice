//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class PluginInitializeFailFactory : Ice.IPluginFactory
{
    public Ice.IPlugin Create(Ice.Communicator communicator, string name, string[] args) => new PluginInitializeFail();

    internal class PluginInitializeFail : Ice.IPlugin
    {
        public void Initialize() => throw new PluginInitializeFailException();

        public void Destroy() => test(false);

        private static void test(bool b)
        {
            if (!b)
            {
                System.Diagnostics.Debug.Assert(false);
                throw new System.Exception();
            }
        }
    }
}
