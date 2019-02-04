//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.packagemd;

import test.Ice.packagemd.Test._InitialDisp;
import test.Ice.packagemd.Test1.C1;
import test.Ice.packagemd.Test1.C2;
import test.Ice.packagemd.Test1.E1;
import test.Ice.packagemd.Test1.E2;
import test.Ice.packagemd.Test1._notify;

public final class InitialI extends _InitialDisp
{
    public Ice.Object
    getTest1C2AsObject(Ice.Current current)
    {
        return new C2();
    }

    @Override
    public C1
    getTest1C2AsC1(Ice.Current current)
    {
        return new C2();
    }

    @Override
    public C2
    getTest1C2AsC2(Ice.Current current)
    {
        return new C2();
    }

    @Override
    public void
    throwTest1E2AsE1(Ice.Current current)
        throws E1
    {
        throw new E2();
    }

    @Override
    public void
    throwTest1E2AsE2(Ice.Current current)
        throws E2
    {
        throw new E2();
    }

    @Override
    public void
    throwTest1Notify(Ice.Current current)
        throws _notify
    {
        throw new _notify();
    }

    public Ice.Object
    getTest2C2AsObject(Ice.Current current)
    {
        return new test.Ice.packagemd.testpkg.Test2.C2();
    }

    @Override
    public test.Ice.packagemd.testpkg.Test2.C1
    getTest2C2AsC1(Ice.Current current)
    {
        return new test.Ice.packagemd.testpkg.Test2.C2();
    }

    @Override
    public test.Ice.packagemd.testpkg.Test2.C2
    getTest2C2AsC2(Ice.Current current)
    {
        return new test.Ice.packagemd.testpkg.Test2.C2();
    }

    @Override
    public void
    throwTest2E2AsE1(Ice.Current current)
        throws test.Ice.packagemd.testpkg.Test2.E1
    {
        throw new test.Ice.packagemd.testpkg.Test2.E2();
    }

    @Override
    public void
    throwTest2E2AsE2(Ice.Current current)
        throws test.Ice.packagemd.testpkg.Test2.E2
    {
        throw new test.Ice.packagemd.testpkg.Test2.E2();
    }

    public Ice.Object
    getTest3C2AsObject(Ice.Current current)
    {
        return new test.Ice.packagemd.modpkg.Test3.C2();
    }

    @Override
    public test.Ice.packagemd.modpkg.Test3.C1
    getTest3C2AsC1(Ice.Current current)
    {
        return new test.Ice.packagemd.modpkg.Test3.C2();
    }

    @Override
    public test.Ice.packagemd.modpkg.Test3.C2
    getTest3C2AsC2(Ice.Current current)
    {
        return new test.Ice.packagemd.modpkg.Test3.C2();
    }

    @Override
    public void
    throwTest3E2AsE1(Ice.Current current)
        throws test.Ice.packagemd.modpkg.Test3.E1
    {
        throw new test.Ice.packagemd.modpkg.Test3.E2();
    }

    @Override
    public void
    throwTest3E2AsE2(Ice.Current current)
        throws test.Ice.packagemd.modpkg.Test3.E2
    {
        throw new test.Ice.packagemd.modpkg.Test3.E2();
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
