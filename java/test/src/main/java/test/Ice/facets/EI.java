//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
