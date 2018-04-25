// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
