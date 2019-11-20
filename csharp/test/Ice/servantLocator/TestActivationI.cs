//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace servantLocator
    {
        public sealed class TestActivationI : Test.TestActivation
        {
            public void activateServantLocator(bool activate, Ice.Current current)
            {
                if (activate)
                {
                    current.adapter.AddServantLocator(new ServantLocatorI(""), "");
                    current.adapter.AddServantLocator(new ServantLocatorI("category"), "category");
                }
                else
                {
                    var locator = current.adapter.RemoveServantLocator("");
                    locator.deactivate("");
                    locator = current.adapter.RemoveServantLocator("category");
                    locator.deactivate("category");
                }
            }
        }
    }
}
