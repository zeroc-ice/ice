// Copyright (c) ZeroC, Inc.

package test.Ice.threadPoolPriority;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.PropertyException;

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

        out.print("testing invalid thread priority... ");
        out.flush();
        Properties properties = new Properties();
        properties.setProperty("Ice.ThreadPool.Client.ThreadPriority", "11");
        try (Communicator communicator = initialize(properties)) {
            test(false);
        } catch (PropertyException ex) {
            // expected: 11 is outside the allowed range [1, 10]
        }
        out.println("ok");
    }
}
