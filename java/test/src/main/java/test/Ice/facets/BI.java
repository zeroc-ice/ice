//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.facets;

import test.Ice.facets.Test.B;

public final class BI implements B
{
    public BI()
    {
    }

    @Override
    public String callA(com.zeroc.Ice.Current current)
    {
        return "A";
    }

    @Override
    public String callB(com.zeroc.Ice.Current current)
    {
        return "B";
    }
}
