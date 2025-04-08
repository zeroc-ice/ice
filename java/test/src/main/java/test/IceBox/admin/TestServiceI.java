// Copyright (c) ZeroC, Inc.

package test.IceBox.admin;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.NativePropertiesAdmin;
import com.zeroc.Ice.Object;
import com.zeroc.IceBox.Service;

public class TestServiceI implements Service {
    public TestServiceI(Communicator serviceManagerCommunicator) {
        TestFacetI facet = new TestFacetI();

        // Install a custom admin facet.
        serviceManagerCommunicator.addAdminFacet(facet, "TestFacet");

        // The TestFacetI servant also implements java.util.function.Consumer<java.util.Map<String,
        // String>>.
        // Set the callback on the admin facet.
        Object propFacet =
            serviceManagerCommunicator.findAdminFacet("IceBox.Service.TestService.Properties");
        if (propFacet != null) {
            NativePropertiesAdmin admin =
                (NativePropertiesAdmin) propFacet;
            admin.addUpdateCallback(facet);
        }
    }

    @Override
    public void start(String name, Communicator communicator, String[] args) {
    }

    @Override
    public void stop() {
    }
}
