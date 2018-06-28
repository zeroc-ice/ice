// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Ice;
using test.Ice.packagemd.modpkg.Test3;
using test.Ice.packagemd.Test1;
using test.Ice.packagemd.testpkg.Test2;

public class InitialI : Test.InitialDisp_
{
    public override test.Ice.packagemd.Test1.C1 getTest1C2AsC1(Current current = null)
    {
        return new test.Ice.packagemd.Test1.C2();
    }

    public override test.Ice.packagemd.Test1.C2 getTest1C2AsC2(Current current = null)
    {
        return new test.Ice.packagemd.Test1.C2();
    }

    public override test.Ice.packagemd.testpkg.Test2.C1 getTest2C2AsC1(Current current = null)
    {
        return new test.Ice.packagemd.testpkg.Test2.C2();
    }

    public override test.Ice.packagemd.testpkg.Test2.C2 getTest2C2AsC2(Current current = null)
    {
        return new test.Ice.packagemd.testpkg.Test2.C2();
    }

    public override test.Ice.packagemd.modpkg.Test3.C1 getTest3C2AsC1(Current current = null)
    {
        return new test.Ice.packagemd.modpkg.Test3.C2();
    }

    public override test.Ice.packagemd.modpkg.Test3.C2 getTest3C2AsC2(Current current = null)
    {
        return new test.Ice.packagemd.modpkg.Test3.C2();
    }

    public override void shutdown(Current current = null)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public override void throwTest1E2AsE1(Current current = null)
    {
        throw new test.Ice.packagemd.Test1.E2();
    }

    public override void throwTest1E2AsE2(Current current = null)
    {
        throw new test.Ice.packagemd.Test1.E2();
    }

    public override void throwTest1Notify(Current current = null)
    {
        throw new test.Ice.packagemd.Test1.@notify();
    }

    public override void throwTest2E2AsE1(Current current = null)
    {
        throw new test.Ice.packagemd.testpkg.Test2.E2();
    }

    public override void throwTest2E2AsE2(Current current = null)
    {
        throw new test.Ice.packagemd.testpkg.Test2.E2();
    }

    public override void throwTest3E2AsE1(Current current = null)
    {
        throw new test.Ice.packagemd.modpkg.Test3.E2();
    }

    public override void throwTest3E2AsE2(Current current = null)
    {
        throw new test.Ice.packagemd.modpkg.Test3.E2();
    }
}
