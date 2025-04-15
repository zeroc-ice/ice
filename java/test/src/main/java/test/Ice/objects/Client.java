// Copyright (c) ZeroC, Inc.

package test.Ice.objects;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Value;
import com.zeroc.Ice.ValueFactory;

import test.Ice.objects.Test.InitialPrx;
import test.TestHelper;

public class Client extends TestHelper {
    private static class MyValueFactory implements ValueFactory {
        @Override
        public Value create(String type) {
            if ("::Test::B".equals(type)) {
                return new BI();
            } else if ("::Test::C".equals(type)) {
                return new CI();
            } else if ("::Test::D".equals(type)) {
                return new DI();
            }
            assert false; // Should never be reached
            return null;
        }
    }

    public void run(String[] args) {
        Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.objects");
        properties.setProperty("Ice.MessageSizeMax", "2048"); // Needed on some Android versions
        try (Communicator communicator = initialize(properties)) {
            ValueFactory factory = new MyValueFactory();
            communicator.getValueFactoryManager().add(factory, "::Test::B");
            communicator.getValueFactoryManager().add(factory, "::Test::C");
            communicator.getValueFactoryManager().add(factory, "::Test::D");

            InitialPrx initial = AllTests.allTests(this);
            initial.shutdown();
        }
    }
}
