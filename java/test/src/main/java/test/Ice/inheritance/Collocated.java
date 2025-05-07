// Copyright (c) ZeroC, Inc.

package test.Ice.inheritance;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.TestHelper;

public class Collocated extends TestHelper {
    @Override
    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new InitialI(adapter), Util.stringToIdentity("initial"));
            AllTests.allTests(this);
        }
    }
}
