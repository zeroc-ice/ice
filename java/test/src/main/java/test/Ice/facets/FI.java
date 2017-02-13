// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test.F;

public final class FI implements F
{
    public FI()
    {
    }

    @Override
    public String callE(com.zeroc.Ice.Current current)
    {
        return "E";
    }

    @Override
    public String callF(com.zeroc.Ice.Current current)
    {
        return "F";
    }
}
