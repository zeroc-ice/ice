// Copyright (c) ZeroC, Inc.

package test.IceBox.configuration;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;
import com.zeroc.IceBox.Service;

public class TestServiceI implements Service {
    @Override
    public void start(String name, Communicator communicator, String[] args) {
        communicator.getProperties().setProperty("Ice.Package.Test", "test.IceBox.configuration");

        ObjectAdapter adapter = communicator.createObjectAdapter(name + "OA");
        adapter.add(new TestI(args), Util.stringToIdentity("test"));
        adapter.activate();
    }

    @Override
    public void stop() {
    }
}
