// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test.C;

public final class CI implements C
{
    public CI()
    {
    }

    @Override
    public String callA(com.zeroc.Ice.Current current)
    {
        return "A";
    }

    @Override
    public String callC(com.zeroc.Ice.Current current)
    {
        return "C";
    }
}
