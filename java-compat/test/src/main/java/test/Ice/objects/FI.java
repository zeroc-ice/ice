//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.objects;

import test.Ice.objects.Test.E;
import test.Ice.objects.Test.F;

public final class FI extends F
{
    public FI()
    {
    }

    public FI(E e)
    {
        super(e, e);
    }

    public boolean checkValues()
    {
        return e1 != null && e1 == e2;
    }
}
