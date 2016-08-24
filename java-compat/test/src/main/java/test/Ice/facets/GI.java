// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.facets;

import test.Ice.facets.Test._GDisp;

public final class GI extends _GDisp
{
    public
    GI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    @Override
    public String
    callG(Ice.Current current)
    {
        return "G";
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        _communicator.shutdown();
    }

    private Ice.Communicator _communicator;
}
