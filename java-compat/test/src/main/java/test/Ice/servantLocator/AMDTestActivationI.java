// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.servantLocator;

import test.Ice.servantLocator.Test._TestActivationDisp;

public final class AMDTestActivationI extends _TestActivationDisp
{
    @Override
    public void activateServantLocator(boolean activate, Ice.Current current)
    {
        if(activate)
        {
            current.adapter.addServantLocator(new AMDServantLocatorI(""), "");
            current.adapter.addServantLocator(new AMDServantLocatorI("category"), "category");
        }
        else
        {
            Ice.ServantLocator locator = current.adapter.removeServantLocator("");
            locator.deactivate("");
            locator = current.adapter.removeServantLocator("category");
            locator.deactivate("category");
        }
    }
}
