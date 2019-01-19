//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
