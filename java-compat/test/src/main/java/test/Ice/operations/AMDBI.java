//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.operations;

import Ice.Current;
import test.Ice.operations.AMD.M.*;

import java.util.*;

public final class AMDBI extends B
{
    @Override
    public void
    opB_async(AMD_B_opB cb, Ice.Current current)
    {
        cb.ice_response();
    }

    @Override
    public void
    opIntf_async(AMD_Intf_opIntf cb, Ice.Current current)
    {
        cb.ice_response();
    }
}
