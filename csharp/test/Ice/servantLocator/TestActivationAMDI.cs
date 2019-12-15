//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace servantLocator
    {
        namespace AMD
        {
            public sealed class TestActivationI : Test.TestActivation
            {
                public void activateServantLocator(bool activate, Ice.Current current)
                {
                    if (activate)
                    {
                        current.Adapter.AddServantLocator(new ServantLocatorI(""), "");
                        current.Adapter.AddServantLocator(new ServantLocatorI("category"), "category");
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
    }
}
