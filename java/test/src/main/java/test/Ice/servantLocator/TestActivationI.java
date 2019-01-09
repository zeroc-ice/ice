// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.servantLocator;

import test.Ice.servantLocator.Test.TestActivation;

public final class TestActivationI implements TestActivation
{
    @Override
    public void activateServantLocator(boolean activate, com.zeroc.Ice.Current current)
    {
        if(activate)
        {
            current.adapter.addServantLocator(new ServantLocatorI(""), "");
            current.adapter.addServantLocator(new ServantLocatorI("category"), "category");
        }
        else
        {
            com.zeroc.Ice.ServantLocator locator = current.adapter.removeServantLocator("");
            locator.deactivate("");
            locator = current.adapter.removeServantLocator("category");
            locator.deactivate("category");
        }
    }
}
