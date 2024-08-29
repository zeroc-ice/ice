// Copyright (c) ZeroC, Inc.

package test.Ice.maxDispatches;

import com.zeroc.Ice.Current;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import test.Ice.maxDispatches.Test.TestIntf;

class TestIntfI implements TestIntf {
  private int _dispatchCount;
  private int _maxDispatchCount;

  private final ScheduledExecutorService _scheduledExecutorService =
      new ScheduledThreadPoolExecutor(1);

  @Override
  public CompletionStage<Void> opAsync(Current current) {
    synchronized (this) {
      _dispatchCount++;
      _maxDispatchCount = Math.max(_maxDispatchCount, _dispatchCount);
    }

    var future = new CompletableFuture<Void>();

    // Decrement dispatchCount in 50ms
    _scheduledExecutorService.schedule(
        () -> {
          synchronized (this) {
            _dispatchCount--;
          }
          future.complete(null);
        },
        50,
        java.util.concurrent.TimeUnit.MILLISECONDS);

    return future;
  }

  @Override
  public synchronized int resetMaxConcurrentDispatches(Current current) {
    int result = _maxDispatchCount;
    _maxDispatchCount = 0;
    return result;
  }

  @Override
  public void shutdown(Current current) {
    current.adapter.getCommunicator().shutdown();
  }
}
