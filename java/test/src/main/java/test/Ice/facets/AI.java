// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test.A;

public final class AI implements A
{
    public AI()
    {
    }

    @Override
    public String callA(com.zeroc.Ice.Current current)
    {
        return "A";
    }
}
