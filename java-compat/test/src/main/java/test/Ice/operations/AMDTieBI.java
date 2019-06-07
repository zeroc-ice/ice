//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.operations;

import Ice.Current;
import test.Ice.operations.AMD.M.*;

import java.util.*;

public final class AMDTieBI implements _BOperations
{
    @Override
    synchronized public void
    opB_async(AMD_B_opB cb, Ice.Current current)
    {
        cb.ice_response();
    }

    @Override
    synchronized public void
    opIntf_async(AMD_Intf_opIntf cb, Ice.Current current)
    {
        cb.ice_response();
    }
}
