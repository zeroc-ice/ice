// Copyright (c) ZeroC, Inc.

package test.Ice.servantLocator;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ServantLocator;

import test.Ice.servantLocator.Test.TestActivation;

public final class AMDTestActivationI implements TestActivation {
    @Override
    public void activateServantLocator(boolean activate, Current current) {
        if (activate) {
            current.adapter.addServantLocator(new AMDServantLocatorI(""), "");
            current.adapter.addServantLocator(new AMDServantLocatorI("category"), "category");
        } else {
            ServantLocator locator = current.adapter.removeServantLocator("");
            locator.deactivate("");
            locator = current.adapter.removeServantLocator("category");
            locator.deactivate("category");
        }
    }
}
