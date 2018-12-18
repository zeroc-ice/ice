// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.servantLocator;

import test.Ice.servantLocator.Test.TestActivation;

public final class AMDTestActivationI implements TestActivation
{
    @Override
    public void activateServantLocator(boolean activate, com.zeroc.Ice.Current current)
    {
        if(activate)
        {
            current.adapter.addServantLocator(new AMDServantLocatorI(""), "");
            current.adapter.addServantLocator(new AMDServantLocatorI("category"), "category");
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
