// Copyright (c) ZeroC, Inc.

package test.Ice.middleware;

import test.Ice.middleware.Test.*;

final class MyObjectI implements MyObject {
  @Override
  public String getName(com.zeroc.Ice.Current current) {
    return "Foo";
  }
}
