//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA.CAPrx;
import test.Ice.inheritance.Test.MB.CB;
import test.Ice.inheritance.Test.MB.CBPrx;

public final class CBI extends CB
{
    public
    CBI()
    {
    }

    @Override
    public CAPrx
    caop(CAPrx p, Ice.Current current)
    {
        return p;
    }

    @Override
    public CBPrx
    cbop(CBPrx p, Ice.Current current)
    {
        return p;
    }
}
