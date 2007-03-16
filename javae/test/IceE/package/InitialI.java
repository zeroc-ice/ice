// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class InitialI extends Test._InitialDisp
{
    public void
    throwTest1E2AsE1(Ice.Current __current)
        throws Test1.E1
    {
        throw new Test1.E2();
    }

    public void
    throwTest1E2AsE2(Ice.Current __current)
        throws Test1.E2
    {
        throw new Test1.E2();
    }

    public void
    throwTest1Notify(Ice.Current __current)
        throws Test1._notify
    {
        throw new Test1._notify();
    }

    public void
    throwTest2E2AsE1(Ice.Current __current)
        throws testpkg.Test2.E1
    {
        throw new testpkg.Test2.E2();
    }

    public void
    throwTest2E2AsE2(Ice.Current __current)
        throws testpkg.Test2.E2
    {
        throw new testpkg.Test2.E2();
    }

    public void
    throwTest3E2AsE1(Ice.Current __current)
        throws testpkg.Test3.E1
    {
        throw new testpkg.Test3.E2();
    }

    public void
    throwTest3E2AsE2(Ice.Current __current)
        throws testpkg.Test3.E2
    {
        throw new testpkg.Test3.E2();
    }

    public void
    shutdown(Ice.Current __current)
    {
        __current.adapter.getCommunicator().shutdown();
    }
}
