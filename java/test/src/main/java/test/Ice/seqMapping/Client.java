// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;

import test.Ice.seqMapping.Test.*;
import test.TestHelper;

import java.io.PrintWriter;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        PrintWriter out = getWriter();

        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.seqMapping");
        try (Communicator communicator = initialize(properties)) {
            MyClassPrx myClass = AllTests.allTests(this, false);

            out.print("shutting down server... ");
            out.flush();
            myClass.shutdown();
            out.println("ok");
        }
    }
}
