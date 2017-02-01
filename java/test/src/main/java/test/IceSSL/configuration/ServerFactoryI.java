// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceSSL.configuration;
import test.IceSSL.configuration.Test.ServerPrx;
import test.IceSSL.configuration.Test.ServerFactory;

class ServerFactoryI implements ServerFactory
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public ServerFactoryI(String defaultDir)
    {
        _defaultDir = defaultDir;
    }

    @Override
    public ServerPrx createServer(java.util.Map<String, String> props, com.zeroc.Ice.Current current)
    {
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.properties = com.zeroc.Ice.Util.createProperties();
        for(java.util.Map.Entry<String, String> i : props.entrySet())
        {
            initData.properties.setProperty(i.getKey(), i.getValue());
        }
        initData.properties.setProperty("IceSSL.DefaultDir", _defaultDir);
        com.zeroc.Ice.Communicator communicator = com.zeroc.Ice.Util.initialize(initData);
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("ServerAdapter", "ssl");
        ServerI server = new ServerI(communicator);
        com.zeroc.Ice.ObjectPrx obj = adapter.addWithUUID(server);
        _servers.put(obj.ice_getIdentity(), server);
        adapter.activate();

        return ServerPrx.uncheckedCast(obj);
    }

    @Override
    public void destroyServer(ServerPrx srv, com.zeroc.Ice.Current current)
    {
        com.zeroc.Ice.Identity key = srv.ice_getIdentity();
        if(_servers.containsKey(key))
        {
            ServerI server = _servers.get(key);
            server.destroy();
            _servers.remove(key);
        }
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        test(_servers.size() == 0);
        current.adapter.getCommunicator().shutdown();
    }

    private String _defaultDir;
    private java.util.Map<com.zeroc.Ice.Identity, ServerI> _servers =
        new java.util.HashMap<com.zeroc.Ice.Identity, ServerI>();
}
