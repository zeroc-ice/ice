// Copyright (c) ZeroC, Inc.

package test.Ice.objects;

import test.Ice.objects.Test.Initial;

public class Collocated extends test.TestHelper {
    private static class MyValueFactory implements com.zeroc.Ice.ValueFactory {
        @Override
        public com.zeroc.Ice.Value create(String type) {
            if (type.equals("::Test::B")) {
                return new BI();
            } else if (type.equals("::Test::C")) {
                return new CI();
            } else if (type.equals("::Test::D")) {
                return new DI();
            }

            assert (false); // Should never be reached
            return null;
        }
    }

    public void run(String[] args) {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.objects");
        properties.setProperty("Ice.Warn.Dispatch", "0");

        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            com.zeroc.Ice.ValueFactory factory = new MyValueFactory();
            communicator.getValueFactoryManager().add(factory, "::Test::B");
            communicator.getValueFactoryManager().add(factory, "::Test::C");
            communicator.getValueFactoryManager().add(factory, "::Test::D");

            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Initial initial = new InitialI(adapter);
            adapter.add(initial, com.zeroc.Ice.Util.stringToIdentity("initial"));
            adapter.add(new F2I(), com.zeroc.Ice.Util.stringToIdentity("F21"));
            UnexpectedObjectExceptionTestI object = new UnexpectedObjectExceptionTestI();
            adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("uoet"));
            AllTests.allTests(this);
            //
            // We must call shutdown even in the collocated case for cyclic dependency cleanup.
            //
            initial.shutdown(null);
        }
    }
}
