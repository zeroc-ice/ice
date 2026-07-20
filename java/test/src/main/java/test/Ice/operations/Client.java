// Copyright (c) ZeroC, Inc.

package test.Ice.operations;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ModuleToPackageSliceLoader;

import test.Ice.operations.Test.MyInterfacePrx;
import test.TestHelper;

import java.io.PrintWriter;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.sliceLoader = new ModuleToPackageSliceLoader("::Test", "test.Ice.operations.Test");
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");

        PrintWriter out = getWriter();
        try (Communicator communicator = initialize(initData)) {
            MyInterfacePrx myInterface = AllTests.allTests(this);

            out.print("testing server shutdown... ");
            out.flush();
            myInterface.shutdown();
            try {
                myInterface.ice_invocationTimeout(100)
                    .ice_ping(); // Use timeout to speed up testing on Windows
                throw new RuntimeException();
            } catch (LocalException ex) {
                out.println("ok");
            }
        }
    }
}
