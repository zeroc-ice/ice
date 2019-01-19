//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA.CA;
import test.Ice.inheritance.Test.MA.CAPrx;

public final class CAI extends CA
{
    public
    CAI()
    {
    }

    @Override
    public CAPrx
    caop(CAPrx p, Ice.Current current)
    {
        return p;
    }
}
