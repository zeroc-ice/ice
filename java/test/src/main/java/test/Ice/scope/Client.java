// Copyright (c) ZeroC, Inc.

package test.Ice.scope;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.TestHelper;

import java.io.PrintWriter;

public class Client extends TestHelper {
    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.scope");
        PrintWriter out = getWriter();
        try (Communicator communicator = initialize(properties)) {
            out.print("test using same type name in different Slice modules... ");
            out.flush();
            AllTests.allTests(this);
            out.println("ok");
        }
    }
}
