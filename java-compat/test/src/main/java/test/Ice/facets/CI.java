// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test._CDisp;

public final class CI extends _CDisp
{
    public
    CI()
    {
    }

    @Override
    public String
    callA(Ice.Current current)
    {
        return "A";
    }

    @Override
    public String
    callC(Ice.Current current)
    {
        return "C";
    }
}
