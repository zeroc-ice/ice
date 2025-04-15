// Copyright (c) ZeroC, Inc.

package test;

import java.io.PrintWriter;

public class TestDriver {
    public static void main(String[] args) {
        int status = 0;
        try {
            String testClass = System.getProperty("test.class");
            Class<?> c = Class.forName(testClass);
            TestHelper helper = (TestHelper) c.getDeclaredConstructor().newInstance();

            // Make the test fail if a thread dies with an unhandled exception.
            Thread.setDefaultUncaughtExceptionHandler(
                (Thread t, Throwable e) -> {
                    PrintWriter out = helper.getWriter();
                    out.println(
                        "!!!!!!! unhandled exception in thread " + t.getName() + ": " + e);
                    e.printStackTrace(out);
                    out.flush();
                    System.exit(1);
                });

            helper.run(args);
            helper.getWriter().flush();
        } catch (Exception ex) {
            ex.printStackTrace();
            status = 1;
        } finally {
            System.gc();
        }
        System.exit(status);
    }
}
