// Copyright (c) ZeroC, Inc.

namespace Ice.servantLocator.AMD;

public sealed class TestActivationI : Test.AsyncTestActivationDisp_
{
    public override Task activateServantLocatorAsync(bool activate, Ice.Current current)
    {
        if (activate)
        {
            current.adapter.addServantLocator(new ServantLocatorI(""), "");
            current.adapter.addServantLocator(new ServantLocatorI("category"), "category");
        }
        else
        {
            ServantLocator locator = current.adapter.removeServantLocator("");
            locator.deactivate("");
            locator = current.adapter.removeServantLocator("category");
            locator.deactivate("category");
        }
        return Task.CompletedTask;
    }
}
