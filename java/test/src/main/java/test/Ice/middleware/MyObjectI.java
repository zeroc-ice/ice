// Copyright (c) ZeroC, Inc.

package test.Ice.middleware;

import com.zeroc.Ice.Current;

import test.Ice.middleware.Test.*;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

final class MyObjectI implements MyObject {
    @Override
    public CompletionStage<String> getNameAsync(Current current) {
        return CompletableFuture.completedFuture("Foo");
    }
}
