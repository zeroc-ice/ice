// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.Ice.seqMapping.Test.MyInterfacePrx;
import test.TestHelper;

import java.io.PrintWriter;

public class Client extends TestHelper {
    @Override
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.seqMapping.Test");
        initData.properties = createTestProperties(args);

        PrintWriter out = getWriter();
        try (Communicator communicator = initialize(initData)) {
            MyInterfacePrx myInterface = AllTests.allTests(this, false);

            out.print("shutting down server... ");
            out.flush();
            myInterface.shutdown();
            out.println("ok");
        }
    }
}
