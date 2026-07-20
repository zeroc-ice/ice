// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping;

import com.zeroc.Ice.Current;

import test.Ice.seqMapping.Serialize.Large;
import test.Ice.seqMapping.Serialize.Small;
import test.Ice.seqMapping.Serialize.Struct;
import test.Ice.seqMapping.Test.MyInterface;

public final class MyInterfaceI implements MyInterface {
    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public MyInterface.OpSerialSmallJavaResult opSerialSmallJava(Small i, Current current) {
        return new MyInterface.OpSerialSmallJavaResult(i, i);
    }

    @Override
    public MyInterface.OpSerialLargeJavaResult opSerialLargeJava(Large i, Current current) {
        return new MyInterface.OpSerialLargeJavaResult(i, i);
    }

    @Override
    public MyInterface.OpSerialStructJavaResult opSerialStructJava(Struct i, Current current) {
        return new MyInterface.OpSerialStructJavaResult(i, i);
    }
}
