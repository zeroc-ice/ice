//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.objects;

import test.Ice.objects.Test.J;

public final class JI extends com.zeroc.Ice.InterfaceByValue
{
    public JI()
    {
        super(J.ice_staticId());
    }
}
