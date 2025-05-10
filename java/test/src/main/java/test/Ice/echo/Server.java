// Copyright (c) ZeroC, Inc.

package test.Ice.echo;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.Ice.echo.Test.Echo;
import test.TestHelper;

public class Server extends TestHelper {
    class EchoI implements Echo {
        public EchoI(BlobjectI blob) {
            _blob = blob;
        }

        @Override
        public void startBatch(Current current) {
            _blob.startBatch();
        }

        @Override
        public void flushBatch(Current current) {
            _blob.flushBatch();
        }

        @Override
        public void shutdown(Current current) {
            current.adapter.getCommunicator().shutdown();
        }

        private final BlobjectI _blob;
    }

    public void run(String[] args) {
        try (Communicator communicator = initialize(args)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            BlobjectI blob = new BlobjectI();
            adapter.addDefaultServant(blob, "");
            adapter.add(new EchoI(blob), Util.stringToIdentity("__echo"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }
}
