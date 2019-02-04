//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.binding;
import test.Ice.binding.Test._TestIntfDisp;

public class TestI extends _TestIntfDisp
{
    TestI()
    {
    }

    @Override
    public String
    getAdapterName(Ice.Current current)
    {
        return current.adapter.getName();
    }
}
