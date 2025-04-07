// Copyright (c) ZeroC, Inc.

package test.Ice.admin;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectPrx;

import test.Ice.admin.Test.*;

import java.util.Map;
import java.util.function.Consumer;

public class RemoteCommunicatorI
        implements RemoteCommunicator, Consumer<Map<String, String>> {
    RemoteCommunicatorI(Communicator communicator) {
        _communicator = communicator;
    }

    @Override
    public ObjectPrx getAdmin(Current current) {
        return _communicator.getAdmin();
    }

    @Override
    public synchronized Map<String, String> getChanges(Current current) {
        return _changes;
    }

    @Override
    public void print(String message, Current current) {
        _communicator.getLogger().print(message);
    }

    @Override
    public void trace(String category, String message, Current current) {
        _communicator.getLogger().trace(category, message);
    }

    @Override
    public void warning(String message, Current current) {
        _communicator.getLogger().warning(message);
    }

    @Override
    public void error(String message, Current current) {
        _communicator.getLogger().error(message);
    }

    @Override
    public void shutdown(Current current) {
        _communicator.shutdown();
    }

    @Override
    public void waitForShutdown(Current current) {
        //
        // Note that we are executing in a thread of the *main* communicator, not the one that is
        // being shut down.
        //
        _communicator.waitForShutdown();
    }

    @Override
    public void destroy(Current current) {
        _communicator.destroy();
    }

    @Override
    public synchronized void accept(Map<String, String> changes) {
        _changes = changes;
    }

    private final Communicator _communicator;
    private Map<String, String> _changes;
}
