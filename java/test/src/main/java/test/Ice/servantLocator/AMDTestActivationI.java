// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
