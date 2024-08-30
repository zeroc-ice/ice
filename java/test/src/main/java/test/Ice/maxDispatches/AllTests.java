// Copyright (c) ZeroC, Inc.

package test.Ice.maxDispatches;

import com.zeroc.Ice.Communicator;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.concurrent.CompletableFuture;
import test.Ice.maxDispatches.Test.TestIntfPrx;

public class AllTests {
  static void allTests(test.TestHelper helper) {
    Communicator communicator = helper.communicator();
    String proxyString = "test: " + helper.getTestEndpoint();
    var p = TestIntfPrx.createProxy(communicator, proxyString);

    String proxyStringMax10 = "test: " + helper.getTestEndpoint(1);
    var pMax10 = TestIntfPrx.createProxy(communicator, proxyStringMax10);

    String proxyStringMax1 = "test: " + helper.getTestEndpoint(2);
    var pMax1 = TestIntfPrx.createProxy(communicator, proxyStringMax1);

    String proxyStringSerialize = "test: " + helper.getTestEndpoint(3);
    var pSerialize = TestIntfPrx.createProxy(communicator, proxyStringSerialize);

    testMaxDispatches(p, 100, helper.getWriter());
    testMaxDispatches(pMax10, 10, helper.getWriter());
    testMaxDispatches(pMax1, 1, helper.getWriter());

    // Serialize does not limit dispatches with "true" AMD.
    testMaxDispatches(pSerialize, 100, helper.getWriter());

    p.shutdown();
  }

  // Verifies max dispatches is implemented correctly.
  private static void testMaxDispatches(TestIntfPrx p, int maxCount, PrintWriter output) {
    output.write("testing max dispatches max " + maxCount + "... ");
    output.flush();

    var futureList = new ArrayList<CompletableFuture<Void>>();

    for (int i = 0; i < maxCount + 20; ++i) {
      futureList.add(p.opAsync());
    }

    for (int i = 0; i < maxCount + 20; ++i) {
      futureList.get(i).join();
    }

    int maxConcurrentDispatches = p.resetMaxConcurrentDispatches();
    if (maxConcurrentDispatches != maxCount) {
      output.println("failed: max count = " + maxConcurrentDispatches);
    }
    test(maxConcurrentDispatches == maxCount);
    output.println("ok");
  }

  private static void test(boolean b) {
    if (!b) {
      throw new RuntimeException();
    }
  }
}
