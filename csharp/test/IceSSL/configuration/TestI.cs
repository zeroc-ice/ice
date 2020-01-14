//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System;
using System.Collections.Generic;

internal sealed class SSLServer : IServer
{
    internal SSLServer(Ice.Communicator communicator) => _communicator = communicator;

    public void
    noCert(Ice.Current current)
    {
        try
        {
            IceSSL.ConnectionInfo info = (IceSSL.ConnectionInfo)current.Connection.getInfo();
            test(info.Certs == null);
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
            test(info.Verified);
            test(info.Certs.Length == 2 &&
                 info.Certs[0].Subject.Equals(subjectDN) &&
                 info.Certs[0].Issuer.Equals(issuerDN));
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
            test(info.Cipher.Equals(cipher));
        }
        catch (Ice.LocalException)
        {
            test(false);
        }
    }

    internal void destroy() => _communicator.destroy();

    private static void test(bool b)
    {
        if (!b)
        {
            throw new Exception();
        }
    }

    private Ice.Communicator _communicator;
}

internal sealed class ServerFactory : IServerFactory
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new Exception();
        }
    }

    public ServerFactory(string defaultDir) => _defaultDir = defaultDir;

    public IServerPrx createServer(Dictionary<string, string> props, Ice.Current current)
    {
        props["IceSSL.DefaultDir"] = _defaultDir;
        Ice.Communicator communicator = new Ice.Communicator(props);
        Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("ServerAdapter", "ssl");
        var server = new SSLServer(communicator);
        var prx = adapter.Add(server);
        _servers[prx.Identity] = server;
        adapter.Activate();
        return prx;
    }

    public void destroyServer(IServerPrx srv, Ice.Current current)
    {
        SSLServer? server;
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
    private Dictionary<Ice.Identity, SSLServer> _servers = new Dictionary<Ice.Identity, SSLServer>();
}
