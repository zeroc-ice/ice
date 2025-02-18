// Copyright (c) ZeroC, Inc.

package test.Ice.packagemd;

import test.Ice.packagemd.Test.Initial;
import test.Ice.packagemd.Test1.C1;
import test.Ice.packagemd.Test1.C2;
import test.Ice.packagemd.Test1.E1;
import test.Ice.packagemd.Test1.E2;
import test.Ice.packagemd.Test1.notify;

public final class InitialI implements Initial {
    public com.zeroc.Ice.Value getTest1C2AsObject(com.zeroc.Ice.Current current) {
        return new C2();
    }

    @Override
    public C1 getTest1C2AsC1(com.zeroc.Ice.Current current) {
        return new C2();
    }

    @Override
    public C2 getTest1C2AsC2(com.zeroc.Ice.Current current) {
        return new C2();
    }

    @Override
    public void throwTest1E2AsE1(com.zeroc.Ice.Current current) throws E1 {
        throw new E2();
    }

    @Override
    public void throwTest1E2AsE2(com.zeroc.Ice.Current current) throws E2 {
        throw new E2();
    }

    @Override
    public void throwTest1Notify(com.zeroc.Ice.Current current) throws notify {
        throw new notify();
    }

    public com.zeroc.Ice.Value getTest2C2AsObject(com.zeroc.Ice.Current current) {
        return new test.Ice.packagemd.testpkg.Test2.C2();
    }

    @Override
    public test.Ice.packagemd.testpkg.Test2.C1 getTest2C2AsC1(com.zeroc.Ice.Current current) {
        return new test.Ice.packagemd.testpkg.Test2.C2();
    }

    @Override
    public test.Ice.packagemd.testpkg.Test2.C2 getTest2C2AsC2(com.zeroc.Ice.Current current) {
        return new test.Ice.packagemd.testpkg.Test2.C2();
    }

    @Override
    public void throwTest2E2AsE1(com.zeroc.Ice.Current current)
            throws test.Ice.packagemd.testpkg.Test2.E1 {
        throw new test.Ice.packagemd.testpkg.Test2.E2();
    }

    @Override
    public void throwTest2E2AsE2(com.zeroc.Ice.Current current)
            throws test.Ice.packagemd.testpkg.Test2.E2 {
        throw new test.Ice.packagemd.testpkg.Test2.E2();
    }

    public com.zeroc.Ice.Value getTest3C2AsObject(com.zeroc.Ice.Current current) {
        return new test.Ice.packagemd.modpkg.Test3.C2();
    }

    @Override
    public test.Ice.packagemd.modpkg.Test3.C1 getTest3C2AsC1(com.zeroc.Ice.Current current) {
        return new test.Ice.packagemd.modpkg.Test3.C2();
    }

    @Override
    public test.Ice.packagemd.modpkg.Test3.C2 getTest3C2AsC2(com.zeroc.Ice.Current current) {
        return new test.Ice.packagemd.modpkg.Test3.C2();
    }

    @Override
    public void throwTest3E2AsE1(com.zeroc.Ice.Current current)
            throws test.Ice.packagemd.modpkg.Test3.E1 {
        throw new test.Ice.packagemd.modpkg.Test3.E2();
    }

    @Override
    public void throwTest3E2AsE2(com.zeroc.Ice.Current current)
            throws test.Ice.packagemd.modpkg.Test3.E2 {
        throw new test.Ice.packagemd.modpkg.Test3.E2();
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current) {
        current.adapter.getCommunicator().shutdown();
    }
}
