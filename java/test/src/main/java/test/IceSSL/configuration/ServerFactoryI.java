// Copyright (c) ZeroC, Inc.

package test.IceSSL.configuration;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.Util;

import test.IceSSL.configuration.Test.ServerFactory;
import test.IceSSL.configuration.Test.ServerPrx;

import java.util.HashMap;
import java.util.Map;

class ServerFactoryI implements ServerFactory {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public ServerFactoryI(String defaultDir) {
        _defaultDir = defaultDir;
    }

    @Override
    public ServerPrx createServer(
            Map<String, String> props, Current current) {
        InitializationData initData = new InitializationData();
        initData.properties = new Properties();
        for (Map.Entry<String, String> i : props.entrySet()) {
            initData.properties.setProperty(i.getKey(), i.getValue());
        }
        initData.properties.setProperty("IceSSL.DefaultDir", _defaultDir);
        Communicator communicator = Util.initialize(initData);
        ObjectAdapter adapter =
            communicator.createObjectAdapterWithEndpoints("ServerAdapter", "ssl");
        ServerI server = new ServerI(communicator);
        ObjectPrx obj = adapter.addWithUUID(server);
        _servers.put(obj.ice_getIdentity(), server);
        adapter.activate();

        return ServerPrx.uncheckedCast(obj);
    }

    @Override
    public void destroyServer(ServerPrx srv, Current current) {
        Identity key = srv.ice_getIdentity();
        if (_servers.containsKey(key)) {
            ServerI server = _servers.get(key);
            server.destroy();
            _servers.remove(key);
        }
    }

    @Override
    public void shutdown(Current current) {
        test(_servers.isEmpty());
        current.adapter.getCommunicator().shutdown();
    }

    private final String _defaultDir;
    private final Map<Identity, ServerI> _servers =
        new HashMap<Identity, ServerI>();
}
