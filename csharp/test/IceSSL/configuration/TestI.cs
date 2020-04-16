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
            var info = (IceSSL.ConnectionInfo)current.Connection!.GetConnectionInfo();
            TestHelper.Assert(info.Certs == null);
        }
        catch (Exception)
        {
            TestHelper.Assert(false);
        }
    }

    public void
    checkCert(string subjectDN, string issuerDN, Ice.Current current)
    {
        try
        {
            var info = (IceSSL.ConnectionInfo)current.Connection!.GetConnectionInfo();
            TestHelper.Assert(info.Verified);
            TestHelper.Assert(info.Certs != null &&
                info.Certs.Length == 2 &&
                info.Certs[0].Subject.Equals(subjectDN) &&
                info.Certs[0].Issuer.Equals(issuerDN));
        }
        catch (Exception)
        {
            TestHelper.Assert(false);
        }
    }

    public void
    checkCipher(string cipher, Ice.Current current)
    {
        try
        {
            var info = (IceSSL.ConnectionInfo)current.Connection!.GetConnectionInfo();
            TestHelper.Assert(info.Cipher != null && info.Cipher.Equals(cipher));
        }
        catch (Exception)
        {
            TestHelper.Assert(false);
        }
    }

    internal void Destroy() => _communicator.Destroy();

    private readonly Ice.Communicator _communicator;
}

internal sealed class ServerFactory : IServerFactory
{
    public ServerFactory(string defaultDir) => _defaultDir = defaultDir;

    public IServerPrx createServer(Dictionary<string, string> props, Ice.Current current)
    {
        props["IceSSL.DefaultDir"] = _defaultDir;
        var communicator = new Ice.Communicator(props);
        Ice.ObjectAdapter adapter = communicator.CreateObjectAdapterWithEndpoints("ServerAdapter", "ssl");
        var server = new SSLServer(communicator);
        var prx = adapter.AddWithUUID(server, IServerPrx.Factory);
        _servers[prx.Identity] = server;
        adapter.Activate();
        return prx;
    }

    public void destroyServer(IServerPrx? srv, Ice.Current current)
    {
        if (_servers.TryGetValue(srv!.Identity, out SSLServer? server))
        {
            server.Destroy();
            _servers.Remove(srv.Identity);
        }
    }

    public void shutdown(Ice.Current current)
    {
        TestHelper.Assert(_servers.Count == 0);
        current.Adapter.Communicator.Shutdown();
    }

    private readonly string _defaultDir;
    private Dictionary<Ice.Identity, SSLServer> _servers = new Dictionary<Ice.Identity, SSLServer>();
}
