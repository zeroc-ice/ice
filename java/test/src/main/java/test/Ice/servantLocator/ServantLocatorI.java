// Copyright (c) ZeroC, Inc.

package test.Ice.servantLocator;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.ServantLocator;
import com.zeroc.Ice.SocketException;
import com.zeroc.Ice.UnknownException;
import com.zeroc.Ice.UnknownLocalException;
import com.zeroc.Ice.UnknownUserException;
import com.zeroc.Ice.UserException;

import test.Ice.servantLocator.Test.TestImpossibleException;
import test.Ice.servantLocator.Test.TestIntfUserException;

public final class ServantLocatorI implements ServantLocator {
    public ServantLocatorI(String category) {
        _category = category;
        _deactivated = false;
        _requestId = -1;
    }

    @SuppressWarnings("deprecation")
    @Override
    protected synchronized void finalize() throws Throwable {
        test(_deactivated);
    }

    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    @Override
    public ServantLocator.LocateResult locate(Current current) throws UserException {
        synchronized (this) {
            test(!_deactivated);
        }

        test(current.id.category.equals(_category) || _category.isEmpty());

        if ("unknown".equals(current.id.name)) {
            return new ServantLocator.LocateResult();
        }

        if ("invalidReturnValue".equals(current.id.name)
                || "invalidReturnType".equals(current.id.name)) {
            return new ServantLocator.LocateResult();
        }

        test("locate".equals(current.id.name) || "finished".equals(current.id.name));
        if ("locate".equals(current.id.name)) {
            exception(current);
        }

        //
        // Ensure locate() is only called once per request.
        //
        test(_requestId == -1);
        _requestId = current.requestId;

        return new ServantLocator.LocateResult(new TestI(), new Cookie());
    }

    @Override
    public void finished(
            Current current, Object servant, java.lang.Object cookie)
            throws UserException {
        synchronized (this) {
            test(!_deactivated);
        }

        //
        // Ensure finished() is only called once per request.
        //
        test(_requestId == current.requestId);
        _requestId = -1;

        test(current.id.category.equals(_category) || _category.isEmpty());
        test("locate".equals(current.id.name) || "finished".equals(current.id.name));

        if ("finished".equals(current.id.name)) {
            exception(current);
        }

        Cookie co = (Cookie) cookie;
        test("blahblah".equals(co.message()));
    }

    @Override
    public synchronized void deactivate(String category) {
        synchronized (this) {
            test(!_deactivated);

            _deactivated = true;
        }
    }

    private void exception(Current current) throws UserException {
        if ("ice_ids".equals(current.operation)) {
            throw new TestIntfUserException();
        } else if ("requestFailedException".equals(current.operation)) {
            throw new ObjectNotExistException();
        } else if ("unknownUserException".equals(current.operation)) {
            throw new UnknownUserException("reason");
        } else if ("unknownLocalException".equals(current.operation)) {
            throw new UnknownLocalException("reason");
        } else if ("unknownException".equals(current.operation)) {
            throw new UnknownException("reason");
        }
        //
        // User exceptions are checked exceptions in Java, so it's not possible to throw it from the
        // servant locator.
        //
        //      else if(current.operation.equals("userException"))
        //      {
        //          throw new TestIntfUserException();
        //      }
        else if ("localException".equals(current.operation)) {
            throw new SocketException();
        } else if ("javaException".equals(current.operation)) {
            throw new RuntimeException("message");
        } else if ("unknownExceptionWithServantException".equals(current.operation)) {
            throw new UnknownException("reason");
        } else if ("impossibleException".equals(current.operation)) {
            throw new TestIntfUserException(); // Yes, it really is meant to be
            // TestIntfUserException.
        } else if ("intfUserException".equals(current.operation)) {
            throw new TestImpossibleException(); // Yes, it really is meant to be
            // TestImpossibleException.
        } else if ("asyncResponse".equals(current.operation)) {
            throw new TestImpossibleException();
        } else if ("asyncException".equals(current.operation)) {
            throw new TestImpossibleException();
        }
    }

    private boolean _deactivated;
    private final String _category;
    private int _requestId;
}
