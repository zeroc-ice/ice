// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping;

import com.zeroc.Ice.Current;

import test.Ice.seqMapping.Serialize.*;
import test.Ice.seqMapping.Test.*;

public final class MyClassI implements MyClass {
    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public MyClass.OpSerialSmallJavaResult opSerialSmallJava(
            Small i, Current current) {
        return new MyClass.OpSerialSmallJavaResult(i, i);
    }

    @Override
    public MyClass.OpSerialLargeJavaResult opSerialLargeJava(
            Large i, Current current) {
        return new MyClass.OpSerialLargeJavaResult(i, i);
    }

    @Override
    public MyClass.OpSerialStructJavaResult opSerialStructJava(
            Struct i, Current current) {
        return new MyClass.OpSerialStructJavaResult(i, i);
    }
}
