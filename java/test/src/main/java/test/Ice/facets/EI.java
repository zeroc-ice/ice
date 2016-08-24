// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test.E;

public final class EI implements E
{
    public EI()
    {
    }

    @Override
    public String callE(com.zeroc.Ice.Current current)
    {
        return "E";
    }
}
