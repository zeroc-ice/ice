// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.seqMapping;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import test.Ice.seqMapping.AMD.Test.*;
import test.Ice.seqMapping.Serialize.*;

public final class AMDMyClassI implements MyClass
{
    @Override
    public CompletionStage<Void> shutdownAsync(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<MyClass.OpSerialSmallJavaResult> opSerialSmallJavaAsync(Small i,
                                                                                   com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture(new MyClass.OpSerialSmallJavaResult(i, i));
    }

    @Override
    public CompletionStage<MyClass.OpSerialLargeJavaResult> opSerialLargeJavaAsync(Large i,
                                                                                   com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture(new MyClass.OpSerialLargeJavaResult(i, i));
    }

    @Override
    public CompletionStage<MyClass.OpSerialStructJavaResult> opSerialStructJavaAsync(Struct i,
                                                                                     com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture(new MyClass.OpSerialStructJavaResult(i, i));
    }
}
