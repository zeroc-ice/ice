// Copyright (c) ZeroC, Inc.

package test.Ice.objects;

import com.zeroc.Ice.ClassSliceLoader;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.CompositeSliceLoader;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectAdapter;

import test.Ice.objects.Test.Compact;
import test.Ice.objects.Test.CompactExt;
import test.Ice.objects.Test.Initial;
import test.Ice.objects.Test.JavaClass;
import test.Ice.objects.Test.JavaDerivedClass;
import test.TestHelper;

public class Collocated extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.sliceLoader =
            new CompositeSliceLoader(
                typeId -> {
                    return switch (typeId) {
                        case "::Test::B" -> new BI();
                        case "::Test::C" -> new CI();
                        case "::Test::D" -> new DI();
                        default -> null;
                    };
                },
                new ClassSliceLoader(Compact.class, CompactExt.class, JavaDerivedClass.class, JavaClass.class),
                new ModuleToPackageSliceLoader("::Test", "test.Ice.objects.Test"));

        try (Communicator communicator = initialize(initData)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Initial initial = new InitialI(adapter);
            adapter.add(initial, new Identity("initial", ""));
            adapter.add(new F2I(), new Identity("F21", ""));
            UnexpectedObjectExceptionTestI object = new UnexpectedObjectExceptionTestI();
            adapter.add(object, new Identity("uoet", ""));
            AllTests.allTests(this);
            //
            // We must call shutdown even in the collocated case for cyclic dependency cleanup.
            //
            initial.shutdown(null);
        }
    }
}
