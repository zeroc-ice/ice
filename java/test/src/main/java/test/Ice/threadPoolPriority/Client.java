// Copyright (c) ZeroC, Inc.

package test.Ice.threadPoolPriority;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectPrx;

import test.Ice.threadPoolPriority.Test.PriorityPrx;
import test.TestHelper;

import java.io.PrintWriter;

public class Client extends TestHelper {
    public void run(String[] args) {
        PrintWriter out = getWriter();
        try (Communicator communicator = initialize(args)) {
            ObjectPrx object =
                communicator.stringToProxy("test:" + getTestEndpoint(0) + " -t 10000");
            PriorityPrx priority = PriorityPrx.checkedCast(object);
            out.print("testing thread priority... ");
            out.flush();
            int prio = priority.getPriority();
            test(prio == 10);
            out.println("ok");
            priority.shutdown();
        }
    }
}
