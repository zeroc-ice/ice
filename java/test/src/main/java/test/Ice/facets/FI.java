//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.facets;

import test.Ice.facets.Test.F;

public final class FI implements F
{
    public FI()
    {
    }

    @Override
    public String callE(com.zeroc.Ice.Current current)
    {
        return "E";
    }

    @Override
    public String callF(com.zeroc.Ice.Current current)
    {
        return "F";
    }
}
