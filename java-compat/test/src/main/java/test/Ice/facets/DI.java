//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.facets;

import test.Ice.facets.Test._DDisp;

public final class DI extends _DDisp
{
    public
    DI()
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
    callB(Ice.Current current)
    {
        return "B";
    }

    @Override
    public String
    callC(Ice.Current current)
    {
        return "C";
    }

    @Override
    public String
    callD(Ice.Current current)
    {
        return "D";
    }
}
