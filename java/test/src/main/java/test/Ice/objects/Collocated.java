// Copyright (c) ZeroC, Inc.

package test.Ice.objects;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ClassSliceLoader;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;
import com.zeroc.Ice.Value;
import com.zeroc.Ice.ValueFactory;

import test.Ice.objects.Test.Compact;
import test.Ice.objects.Test.CompactExt;
import test.Ice.objects.Test.Initial;
import test.TestHelper;

public class Collocated extends TestHelper {
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
        var initData = new InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.objects");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.sliceLoader = new ClassSliceLoader(Compact.class, CompactExt.class);

        try (Communicator communicator = initialize(initData)) {
            ValueFactory factory = new MyValueFactory();
            communicator.getValueFactoryManager().add(factory, "::Test::B");
            communicator.getValueFactoryManager().add(factory, "::Test::C");
            communicator.getValueFactoryManager().add(factory, "::Test::D");

            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Initial initial = new InitialI(adapter);
            adapter.add(initial, Util.stringToIdentity("initial"));
            adapter.add(new F2I(), Util.stringToIdentity("F21"));
            UnexpectedObjectExceptionTestI object = new UnexpectedObjectExceptionTestI();
            adapter.add(object, Util.stringToIdentity("uoet"));
            AllTests.allTests(this);
            //
            // We must call shutdown even in the collocated case for cyclic dependency cleanup.
            //
            initial.shutdown(null);
        }
    }
}
