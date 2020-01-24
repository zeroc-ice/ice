//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.servantLocator.AMD
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
                locator.Deactivate("");
                locator = current.Adapter.RemoveServantLocator("category");
                locator.Deactivate("category");
            }
        }
    }
}
