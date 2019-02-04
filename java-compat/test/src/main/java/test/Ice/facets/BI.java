//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.facets;

import test.Ice.facets.Test._BDisp;

public final class BI extends _BDisp
{
    public
    BI()
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
}
