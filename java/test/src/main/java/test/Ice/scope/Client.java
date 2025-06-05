// Copyright (c) ZeroC, Inc.

package test.Ice.scope;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.TestHelper;

import java.io.PrintWriter;
import java.util.Map;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader(
            Map.of("::Test", "test.Ice.scope.Test", "::Inner", "test.Ice.scope.Inner"),
            null
        );
        initData.properties = createTestProperties(args);

        PrintWriter out = getWriter();
        try (Communicator communicator = initialize(initData)) {
            out.print("test using same type name in different Slice modules... ");
            out.flush();
            AllTests.allTests(this);
            out.println("ok");
        }
    }
}
