// Copyright (c) ZeroC, Inc.

package test.Ice.invoke;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ServantLocator;

public class ServantLocatorI implements ServantLocator {
    public ServantLocatorI(boolean async) {
        _blobject = new ServantLocator.LocateResult();
        if (async) {
            _blobject.returnValue = new BlobjectAsyncI();
        } else {
            _blobject.returnValue = new BlobjectI();
        }
    }

    @Override
    public ServantLocator.LocateResult locate(Current current) {
        return _blobject;
    }

    @Override
    public void finished(
            Current current, Object servant, java.lang.Object cookie) {}

    @Override
    public void deactivate(String category) {}

    private final ServantLocator.LocateResult _blobject;
}
