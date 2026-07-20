// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping;

import com.zeroc.Ice.Current;

import test.Ice.seqMapping.Serialize.Large;
import test.Ice.seqMapping.Serialize.Small;
import test.Ice.seqMapping.Serialize.Struct;
import test.Ice.seqMapping.Test.AsyncMyInterface;
import test.Ice.seqMapping.Test.MyInterface;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public final class AMDMyInterfaceI implements AsyncMyInterface {
    @Override
    public CompletionStage<Void> shutdownAsync(Current current) {
        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<MyInterface.OpSerialSmallJavaResult> opSerialSmallJavaAsync(Small i, Current current) {
        return CompletableFuture.completedFuture(new MyInterface.OpSerialSmallJavaResult(i, i));
    }

    @Override
    public CompletionStage<MyInterface.OpSerialLargeJavaResult> opSerialLargeJavaAsync(Large i, Current current) {
        return CompletableFuture.completedFuture(new MyInterface.OpSerialLargeJavaResult(i, i));
    }

    @Override
    public CompletionStage<MyInterface.OpSerialStructJavaResult> opSerialStructJavaAsync(Struct i, Current current) {
        return CompletableFuture.completedFuture(new MyInterface.OpSerialStructJavaResult(i, i));
    }
}
