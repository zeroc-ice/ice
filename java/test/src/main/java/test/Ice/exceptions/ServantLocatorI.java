// Copyright (c) ZeroC, Inc.

package test.Ice.exceptions;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ServantLocator;

public final class ServantLocatorI implements ServantLocator {
    @Override
    public ServantLocator.LocateResult locate(Current curr) {
        return new ServantLocator.LocateResult();
    }

    @Override
    public void finished(
            Current curr, Object servant, java.lang.Object cookie) {
    }

    @Override
    public void deactivate(String category) {
    }
}
