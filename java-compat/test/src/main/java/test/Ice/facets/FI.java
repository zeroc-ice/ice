// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test._FDisp;

public final class FI extends _FDisp
{
    public
    FI()
    {
    }

    @Override
    public String
    callE(Ice.Current current)
    {
        return "E";
    }

    @Override
    public String
    callF(Ice.Current current)
    {
        return "F";
    }
}
