// Copyright (c) ZeroC, Inc.

package test.Ice.location;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.LocatorPrx;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.SocketException;
import com.zeroc.Ice.Util;

import test.Ice.location.Test.ServerManager;
import test.TestHelper;

import java.util.ArrayList;
import java.util.List;

public class ServerManagerI implements ServerManager {
    ServerManagerI(ServerLocatorRegistry registry, TestHelper helper) {
        _registry = registry;
        _communicators = new ArrayList<Communicator>();

        _helper = helper;
        _properties = _helper.communicator().getProperties()._clone();
        _properties.setProperty("TestAdapter.AdapterId", "TestAdapter");
        _properties.setProperty("TestAdapter.ReplicaGroupId", "ReplicatedAdapter");
        _properties.setProperty("TestAdapter2.AdapterId", "TestAdapter2");
    }

    @Override
    public void startServer(Current current) {
        for (Communicator c : _communicators) {
            c.waitForShutdown();
            c.destroy();
        }
        _communicators.clear();

        //
        // Simulate a server: create a new communicator and object
        // adapter. The object adapter is started on a system allocated
        // port. The configuration used here contains the Ice.Locator
        // configuration variable. The new object adapter will register
        // its endpoints with the locator and create references containing
        // the adapter id instead of the endpoints.
        //
        Communicator serverCommunicator = _helper.initialize(_properties);
        _communicators.add(serverCommunicator);

        //
        // Use fixed port to ensure that OA re-activation doesn't re-use previous port from
        // another OA (e.g.: TestAdapter2 is re-activated using port of TestAdapter).
        //
        int nRetry = 10;
        while (--nRetry > 0) {
            ObjectAdapter adapter = null;
            ObjectAdapter adapter2 = null;
            try {
                serverCommunicator
                        .getProperties()
                        .setProperty("TestAdapter.Endpoints", _helper.getTestEndpoint(_nextPort++));
                serverCommunicator
                        .getProperties()
                        .setProperty(
                                "TestAdapter2.Endpoints", _helper.getTestEndpoint(_nextPort++));

                adapter = serverCommunicator.createObjectAdapter("TestAdapter");
                adapter2 = serverCommunicator.createObjectAdapter("TestAdapter2");

                ObjectPrx locator =
                        serverCommunicator.stringToProxy("locator:" + _helper.getTestEndpoint(0));
                adapter.setLocator(LocatorPrx.uncheckedCast(locator));
                adapter2.setLocator(LocatorPrx.uncheckedCast(locator));

                Object object = new TestI(adapter, adapter2, _registry);
                _registry.addObject(
                        adapter.add(object, Util.stringToIdentity("test")), null);
                _registry.addObject(
                        adapter.add(object, Util.stringToIdentity("test2")), null);
                adapter.add(object, Util.stringToIdentity("test3"));

                adapter.activate();
                adapter2.activate();
                break;
            } catch (SocketException ex) {
                if (nRetry == 0) {
                    throw ex;
                }

                // Retry, if OA creation fails with EADDRINUSE (this can occur when running with JS
                // web browser clients if the driver uses ports in the same range as this test,
                // ICE-8148)
                if (adapter != null) {
                    adapter.destroy();
                }
                if (adapter2 != null) {
                    adapter2.destroy();
                }
            }
        }
    }

    @Override
    public void shutdown(Current current) {
        for (Communicator c : _communicators) {
            c.destroy();
        }
        current.adapter.getCommunicator().shutdown();
    }

    private final ServerLocatorRegistry _registry;
    private final List<Communicator> _communicators;
    private final Properties _properties;
    private final TestHelper _helper;
    private int _nextPort = 1;
}
