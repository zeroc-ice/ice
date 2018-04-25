// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.seqMapping;

import test.Ice.seqMapping.AMD.Test.*;
import test.Ice.seqMapping.Serialize.*;

public final class AMDMyClassI extends _MyClassDisp
{
    @Override
    public void
    shutdown_async(AMD_MyClass_shutdown cb, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    @Override
    public void
    opSerialSmallJava_async(AMD_MyClass_opSerialSmallJava cb, Small i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    @Override
    public void
    opSerialLargeJava_async(AMD_MyClass_opSerialLargeJava cb, Large i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }

    @Override
    public void opSerialStructJava_async(AMD_MyClass_opSerialStructJava cb, Struct i, Ice.Current current)
    {
        cb.ice_response(i, i);
    }
}
