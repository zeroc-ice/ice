// Copyright (c) ZeroC, Inc.

package test.Ice.maxConnections;

import com.zeroc.Ice.Current;
import test.Ice.maxConnections.Test.TestIntf;

class TestIntfI implements TestIntf {
  @Override
  public void shutdown(Current current) {
    current.adapter.getCommunicator().shutdown();
  }
}
