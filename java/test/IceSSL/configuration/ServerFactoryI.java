// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class ServerFactoryI extends Test._ServerFactoryDisp
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public Test.ServerPrx
    createServer(java.util.Map _props, Ice.Current current)
    {
        // TODO: Fix the parameters and remove the cast below when the Java2 mapping is removed.
        java.util.Map<String, String> props = (java.util.Map<String, String>)_props;
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        java.util.Iterator<java.util.Map.Entry<String, String> > i = props.entrySet().iterator();
        while(i.hasNext())
        {
            java.util.Map.Entry<String, String> e = i.next();
            initData.properties.setProperty(e.getKey(), e.getValue());
        }

        String[] args = new String[0];
        Ice.Communicator communicator = Ice.Util.initialize(args, initData);
        Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("ServerAdapter", "ssl");
        ServerI server = new ServerI(communicator);
        Ice.ObjectPrx obj = adapter.addWithUUID(server);
        _servers.put(obj.ice_getIdentity(), server);
        adapter.activate();

        return Test.ServerPrxHelper.uncheckedCast(obj);
    }

    public void
    destroyServer(Test.ServerPrx srv, Ice.Current current)
    {
        Ice.Identity key = srv.ice_getIdentity();
        if(_servers.containsKey(key))
        {
            ServerI server = _servers.get(key);
            server.destroy();
            _servers.remove(key);
        }
    }

    public void
    shutdown(Ice.Current current)
    {
        test(_servers.size() == 0);
        current.adapter.getCommunicator().shutdown();
    }

    private java.util.Map<Ice.Identity, ServerI> _servers = new java.util.HashMap<Ice.Identity, ServerI>();
}
