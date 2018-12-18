// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
