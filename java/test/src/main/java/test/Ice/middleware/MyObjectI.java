// Copyright (c) ZeroC, Inc.

package test.Ice.middleware;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;
import test.Ice.middleware.Test.*;

final class MyObjectI implements MyObject {
  @Override
  public CompletionStage<String> getNameAsync(com.zeroc.Ice.Current current) {
    return CompletableFuture.completedFuture("Foo");
  }
}
