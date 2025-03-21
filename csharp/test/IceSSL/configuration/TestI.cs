// Copyright (c) ZeroC, Inc.

using System.Collections;
using Test;

internal sealed class ServerI : ServerDisp_
{
    internal ServerI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public override void
    noCert(Ice.Current current)
    {
        try
        {
            Ice.SSL.ConnectionInfo info = (Ice.SSL.ConnectionInfo)current.con.getInfo();
            test(info.certs.Length == 0);
        }
        catch (Ice.LocalException)
        {
            test(false);
        }
    }

    public override void
    checkCert(string subjectDN, string issuerDN, Ice.Current current)
    {
        try
        {
            Ice.SSL.ConnectionInfo info = (Ice.SSL.ConnectionInfo)current.con.getInfo();
            test(info.verified);
            test(info.certs.Length == 1 &&
                 info.certs[0].Subject.Equals(subjectDN) &&
                 info.certs[0].Issuer.Equals(issuerDN));
        }
        catch (Ice.LocalException)
        {
            test(false);
        }
    }

    public override void
    checkCipher(string cipher, Ice.Current current)
    {
        try
        {
            Ice.SSL.ConnectionInfo info = (Ice.SSL.ConnectionInfo)current.con.getInfo();
            test(info.cipher.Equals(cipher));
        }
        catch (Ice.LocalException)
        {
            test(false);
        }
    }

    internal void destroy()
    {
        _communicator.destroy();
    }

    private static void test(bool b) => global::Test.TestHelper.test(b);

    private readonly Ice.Communicator _communicator;
}

internal sealed class ServerFactoryI : ServerFactoryDisp_
{
    private static void test(bool b) => global::Test.TestHelper.test(b);

    public ServerFactoryI(string defaultDir)
    {
        _defaultDir = defaultDir;
    }

    public override ServerPrx createServer(Dictionary<string, string> props, Ice.Current current)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = new Ice.Properties();
        foreach (string key in props.Keys)
        {
            initData.properties.setProperty(key, props[key]);
        }
        initData.properties.setProperty("IceSSL.DefaultDir", _defaultDir);
        string[] args = new string[0];
        Ice.Communicator communicator = Ice.Util.initialize(ref args, initData);
        Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("ServerAdapter", "ssl");
        ServerI server = new ServerI(communicator);
        Ice.ObjectPrx obj = adapter.addWithUUID(server);
        _servers[obj.ice_getIdentity()] = server;
        adapter.activate();
        return ServerPrxHelper.uncheckedCast(obj);
    }

    public override void destroyServer(ServerPrx srv, Ice.Current current)
    {
        Ice.Identity key = srv.ice_getIdentity();
        if (_servers.Contains(key))
        {
            ServerI server = _servers[key] as ServerI;
            server.destroy();
            _servers.Remove(key);
        }
    }

    public override void shutdown(Ice.Current current)
    {
        test(_servers.Count == 0);
        current.adapter.getCommunicator().shutdown();
    }

    private readonly string _defaultDir;
    private readonly Hashtable _servers = new Hashtable();
}
