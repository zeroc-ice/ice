// Copyright (c) ZeroC, Inc.

package test.Ice.middleware;

import test.Ice.middleware.Test.*;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

final class MyObjectI implements MyObject {
    @Override
    public CompletionStage<String> getNameAsync(com.zeroc.Ice.Current current) {
        return CompletableFuture.completedFuture("Foo");
    }
}
