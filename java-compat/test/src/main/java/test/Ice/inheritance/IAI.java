//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA.IAPrx;
import test.Ice.inheritance.Test.MA._IADisp;

public final class IAI extends _IADisp
{
    public
    IAI()
    {
    }

    @Override
    public IAPrx
    iaop(IAPrx p, Ice.Current current)
    {
        return p;
    }
}
