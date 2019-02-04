//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.facets;

import test.Ice.facets.Test._EDisp;

public final class EI extends _EDisp
{
    public
    EI()
    {
    }

    @Override
    public String
    callE(Ice.Current current)
    {
        return "E";
    }
}
