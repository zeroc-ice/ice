//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
