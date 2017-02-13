// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test._ADisp;

public final class AI extends _ADisp
{
    public
    AI()
    {
    }

    @Override
    public String
    callA(Ice.Current current)
    {
        return "A";
    }
}
