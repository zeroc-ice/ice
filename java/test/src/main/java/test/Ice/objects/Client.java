// Copyright (c) ZeroC, Inc.

package test.Ice.objects;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ClassSliceLoader;
import com.zeroc.Ice.CompositeSliceLoader;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Value;

import test.Ice.objects.Test.Compact;
import test.Ice.objects.Test.CompactExt;
import test.Ice.objects.Test.InitialPrx;
import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] args) {
        var initData = new InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.objects");
        initData.properties.setProperty("Ice.MessageSizeMax", "2048"); // Needed on some Android versions
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
                new ClassSliceLoader(Compact.class, CompactExt.class));

        try (Communicator communicator = initialize(initData)) {
            InitialPrx initial = AllTests.allTests(this);
            initial.shutdown();
        }
    }
}
