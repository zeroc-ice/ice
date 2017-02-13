// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test.D;

public final class DI implements D
{
    public DI()
    {
    }

    @Override
    public String callA(com.zeroc.Ice.Current current)
    {
        return "A";
    }

    @Override
    public String callB(com.zeroc.Ice.Current current)
    {
        return "B";
    }

    @Override
    public String callC(com.zeroc.Ice.Current current)
    {
        return "C";
    }

    @Override
    public String callD(com.zeroc.Ice.Current current)
    {
        return "D";
    }
}
