// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping;

import com.zeroc.Ice.Current;

import test.Ice.seqMapping.AMD.Test.*;
import test.Ice.seqMapping.Serialize.*;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public final class AMDMyClassI implements MyClass {
    @Override
    public CompletionStage<Void> shutdownAsync(Current current) {
        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<MyClass.OpSerialSmallJavaResult> opSerialSmallJavaAsync(
            Small i, Current current) {
        return CompletableFuture.completedFuture(new MyClass.OpSerialSmallJavaResult(i, i));
    }

    @Override
    public CompletionStage<MyClass.OpSerialLargeJavaResult> opSerialLargeJavaAsync(
            Large i, Current current) {
        return CompletableFuture.completedFuture(new MyClass.OpSerialLargeJavaResult(i, i));
    }

    @Override
    public CompletionStage<MyClass.OpSerialStructJavaResult> opSerialStructJavaAsync(
            Struct i, Current current) {
        return CompletableFuture.completedFuture(new MyClass.OpSerialStructJavaResult(i, i));
    }
}
