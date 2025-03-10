// Copyright (c) ZeroC, Inc.

namespace Ice.servantLocator
{
        public sealed class TestActivationI : Test.TestActivationDisp_
        {
            public override void activateServantLocator(bool activate, Ice.Current current)
            {
                if (activate)
                {
                    current.adapter.addServantLocator(new ServantLocatorI(""), "");
                    current.adapter.addServantLocator(new ServantLocatorI("category"), "category");
                }
                else
                {
                    var locator = current.adapter.removeServantLocator("");
                    locator.deactivate("");
                    locator = current.adapter.removeServantLocator("category");
                    locator.deactivate("category");
                }
            }
        }
    }

