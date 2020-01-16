//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.servantLocator
{
    public sealed class TestActivation : Test.ITestActivation
    {
        public void activateServantLocator(bool activate, Current current)
        {
            if (activate)
            {
                current.Adapter.AddServantLocator(new ServantLocator(""), "");
                current.Adapter.AddServantLocator(new ServantLocator("category"), "category");
            }
            else
            {
                var locator = current.Adapter.RemoveServantLocator("");
                locator.deactivate("");
                locator = current.Adapter.RemoveServantLocator("category");
                locator.deactivate("category");
            }
        }
    }
}
