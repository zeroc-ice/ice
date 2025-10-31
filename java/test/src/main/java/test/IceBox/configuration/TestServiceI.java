// Copyright (c) ZeroC, Inc.

package test.IceBox.configuration;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.IceBox.Service;

public class TestServiceI implements Service {
    @Override
    public void start(String name, Communicator communicator, String[] args) {
        ObjectAdapter adapter = communicator.createObjectAdapter(name + "OA");
        adapter.add(new TestI(args), new Identity("test", ""));
        adapter.activate();
    }

    @Override
    public void stop() {}
}
