// Copyright (c) ZeroC, Inc.

package test.Ice.admin;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.Logger;
import com.zeroc.Ice.NativePropertiesAdmin;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.Ice.admin.Test.RemoteCommunicatorFactory;
import test.Ice.admin.Test.RemoteCommunicatorPrx;

import java.util.Map;

public class RemoteCommunicatorFactoryI implements RemoteCommunicatorFactory {
    @Override
    public RemoteCommunicatorPrx createCommunicator(
            Map<String, String> props, Current current) {
        //
        // Prepare the property set using the given properties.
        //
        InitializationData initData = new InitializationData();
        initData.classLoader = current.adapter.getCommunicator().getInstance().getClassLoader();
        initData.properties = new Properties();
        for (Map.Entry<String, String> e : props.entrySet()) {
            initData.properties.setProperty(e.getKey(), e.getValue());
        }

        if (initData.properties.getPropertyAsInt("NullLogger") > 0) {
            initData.logger =
                new Logger() {
                    @Override
                    public void print(String message) {}

                    @Override
                    public void trace(String category, String message) {}

                    @Override
                    public void warning(String message) {}

                    @Override
                    public void error(String message) {}

                    @Override
                    public String getPrefix() {
                        return "NullLogger";
                    }

                    @Override
                    public Logger cloneWithPrefix(String prefix) {
                        return this;
                    }
                };
        }

        //
        // Initialize a new communicator.
        //
        Communicator communicator = Util.initialize(initData);

        //
        // Install a custom admin facet.
        //
        communicator.addAdminFacet(new TestFacetI(), "TestFacet");

        //
        // The RemoteCommunicator servant also implements
        // java.util.function.Consumer<java.util.Map<String, String>>.
        // Set the callback on the admin facet.
        //
        RemoteCommunicatorI servant = new RemoteCommunicatorI(communicator);
        Object propFacet = communicator.findAdminFacet("Properties");

        if (propFacet != null) {
            NativePropertiesAdmin admin =
                (NativePropertiesAdmin) propFacet;
            assert admin != null;
            admin.addUpdateCallback(servant);
        }

        ObjectPrx proxy = current.adapter.addWithUUID(servant);
        return RemoteCommunicatorPrx.uncheckedCast(proxy);
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }
}
