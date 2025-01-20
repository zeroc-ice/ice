// Copyright (c) ZeroC, Inc.

package test.Ice.objects;

import test.Ice.objects.Test.InitialPrx;

public class Client extends test.TestHelper {
    private static class MyValueFactory implements com.zeroc.Ice.ValueFactory {
        @Override
        public com.zeroc.Ice.Value create(String type) {
            if (type.equals("::Test::B")) {
                return new BI();
            } else if (type.equals("::Test::C")) {
                return new CI();
            } else if (type.equals("::Test::D")) {
                return new DI();
            } else if (type.equals("::Test::E")) {
                return new EI();
            } else if (type.equals("::Test::F")) {
                return new FI();
            }
            assert (false); // Should never be reached
            return null;
        }
    }

    public void run(String[] args) {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.objects");
        properties.setProperty("Ice.MessageSizeMax", "2048"); // Needed on some Android versions
        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            com.zeroc.Ice.ValueFactory factory = new MyValueFactory();
            communicator.getValueFactoryManager().add(factory, "::Test::B");
            communicator.getValueFactoryManager().add(factory, "::Test::C");
            communicator.getValueFactoryManager().add(factory, "::Test::D");
            communicator.getValueFactoryManager().add(factory, "::Test::E");
            communicator.getValueFactoryManager().add(factory, "::Test::F");

            InitialPrx initial = AllTests.allTests(this);
            initial.shutdown();
        }
    }
}
