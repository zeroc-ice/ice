//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System;
using System.Collections;
using System.Collections.Generic;

internal sealed class ServerI : Test.Server
{
    internal ServerI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public void
    noCert(Ice.Current current)
    {
        try
        {
            IceSSL.ConnectionInfo info = (IceSSL.ConnectionInfo)current.Connection.getInfo();
            test(info.certs == null);
        }
        catch (Ice.LocalException)
        {
            test(false);
        }
    }

    public void
    checkCert(string subjectDN, string issuerDN, Ice.Current current)
    {
        try
        {
            IceSSL.ConnectionInfo info = (IceSSL.ConnectionInfo)current.Connection.getInfo();
            test(info.verified);
            test(info.certs.Length == 2 &&
                 info.certs[0].Subject.Equals(subjectDN) &&
                 info.certs[0].Issuer.Equals(issuerDN));
        }
        catch (Ice.LocalException)
        {
            test(false);
        }
    }

    public void
    checkCipher(string cipher, Ice.Current current)
    {
        try
        {
            IceSSL.ConnectionInfo info = (IceSSL.ConnectionInfo)current.Connection.getInfo();
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

    private static void test(bool b)
    {
        if (!b)
        {
            throw new Exception();
        }
    }

    private Ice.Communicator _communicator;
}

internal sealed class ServerFactoryI : ServerFactory
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new Exception();
        }
    }

    public ServerFactoryI(string defaultDir)
    {
        _defaultDir = defaultDir;
    }

    public ServerPrx createServer(Dictionary<string, string> props, Ice.Current current)
    {
        props["IceSSL.DefaultDir"] = _defaultDir;
        Ice.Communicator communicator = new Ice.Communicator(props);
        Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("ServerAdapter", "ssl");
        ServerI server = new ServerI(communicator);
        var prx = adapter.Add(server);
        _servers[prx.Identity] = server;
        adapter.Activate();
        return prx;
    }

    public void destroyServer(ServerPrx srv, Ice.Current current)
    {
        ServerI? server;
        if (_servers.TryGetValue(srv.Identity, out server))
        {
            server.destroy();
            _servers.Remove(srv.Identity);
        }
    }

    public void shutdown(Ice.Current current)
    {
        test(_servers.Count == 0);
        current.Adapter.Communicator.shutdown();
    }

    private string _defaultDir;
    private Dictionary<Ice.Identity, ServerI> _servers = new Dictionary<Ice.Identity, ServerI>();
}
