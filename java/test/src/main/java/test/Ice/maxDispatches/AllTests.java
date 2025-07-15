// Copyright (c) ZeroC, Inc.

package test.Ice.maxDispatches;

import com.zeroc.Ice.Communicator;

import test.Ice.maxDispatches.Test.ResponderPrx;
import test.Ice.maxDispatches.Test.TestIntfPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.concurrent.CompletableFuture;

public class AllTests {
    static void allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        String proxyString = "test: " + helper.getTestEndpoint(0);
        var p = TestIntfPrx.createProxy(communicator, proxyString);

        String responderString = "responder: " + helper.getTestEndpoint(1);
        var responder = ResponderPrx.createProxy(communicator, responderString);

        String proxyStringMax10 = "test: " + helper.getTestEndpoint(2);
        var pMax10 = TestIntfPrx.createProxy(communicator, proxyStringMax10);

        String proxyStringMax1 = "test: " + helper.getTestEndpoint(3);
        var pMax1 = TestIntfPrx.createProxy(communicator, proxyStringMax1);

        String proxyStringSerialize = "test: " + helper.getTestEndpoint(4);
        var pSerialize = TestIntfPrx.createProxy(communicator, proxyStringSerialize);

        testMaxDispatches(p, responder, 100, helper.getWriter());
        testMaxDispatches(pMax10, responder, 10, helper.getWriter());
        testMaxDispatches(pMax1, responder, 1, helper.getWriter());

        // Serialize does not limit dispatches with "true" AMD.
        testMaxDispatches(pSerialize, responder, 100, helper.getWriter());

        p.shutdown();
    }

    // Verifies max dispatches is implemented correctly.
    private static void testMaxDispatches(
            TestIntfPrx p, ResponderPrx responder, int maxCount, PrintWriter output) {
        output.write("testing max dispatches max " + maxCount + "... ");
        output.flush();

        // Make sure we start fresh
        responder.stop();
        test(responder.pendingResponseCount() == 0);
        p.resetMaxConcurrentDispatches();

        var futureList = new ArrayList<CompletableFuture<Void>>();

        for (int i = 0; i < maxCount + 20; i++) {
            futureList.add(p.opAsync());
        }

        // Wait until the responder gets at least maxCount responses.
        while (responder.pendingResponseCount() < maxCount) {
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }

        responder.start();

        for (int i = 0; i < maxCount + 20; i++) {
            futureList.get(i).join();
        }

        int maxConcurrentDispatches = p.resetMaxConcurrentDispatches();
        test(maxConcurrentDispatches == maxCount);
        output.println("ok");
    }

    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private AllTests() {}
}
