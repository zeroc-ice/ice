// Copyright (c) ZeroC, Inc.

using System.Net.Security;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using Test;

public class PlatformTests
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new Exception();
        }
    }

    private static Ice.Communicator createServer(
        SslServerAuthenticationOptions serverAuthenticationOptions,
        TestHelper helper)
    {
        var communicator = Ice.Util.initialize();
        var adapter = communicator.createObjectAdapterWithEndpoints(
            "ServerAdapter",
            helper.getTestEndpoint(10, "ssl"),
            serverAuthenticationOptions);
        adapter.add(new ServerI(communicator), Ice.Util.stringToIdentity("server"));
        adapter.activate();
        return communicator;
    }

    private static Ice.Communicator createClient(SslClientAuthenticationOptions clientAuthenticationOptions)
    {
        var communicator = Ice.Util.initialize(new Ice.InitializationData
        {
            clientAuthenticationOptions = clientAuthenticationOptions
        });
        return communicator;
    }

    private static void clientValidatesServerUsingValidationCallback(TestHelper helper, string certificatesPath)
    {
        Console.Out.Write("client validates server certificate using validation callback... ");
        Console.Out.Flush();
        using var serverCertificate =
            new X509Certificate2(Path.Combine(certificatesPath, "ca1/server.p12"), "password");
        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = serverCertificate,
        };
        using var serverCommunicator = createServer(serverOptions, helper);

        var clientOptions = new SslClientAuthenticationOptions
        {
            RemoteCertificateValidationCallback =
                (sender, certificate, chain, sslPolicyErrors) => certificate.Subject == serverCertificate.Subject
        };
        using var clientCommunicator = createClient(clientOptions);

        var obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
        obj.ice_ping();
        Console.Out.WriteLine("ok");
    }

    private static void clientValidatesServerUsingSystemTrustedRootCertificates(
        TestHelper helper,
        string certificatesPath)
    {
        Console.Out.Write("client validates server certificate using system trusted root certificates... ");
        Console.Out.Flush();
        var clientOptions = new SslClientAuthenticationOptions
        {
            TargetHost = "zeroc.com"
        };
        using var clientCommunicator = createClient(clientOptions);
        var obj = Ice.ObjectPrxHelper.createProxy(
            clientCommunicator,
            "Glacier2/router:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2");
        obj.ice_ping();
        Console.Out.WriteLine("ok");
    }

    private static void clientRejectServerUsingValidationCallback(TestHelper helper, string certificatesPath)
    {
        Console.Out.Write("client rejects server certificate using validation callback... ");
        Console.Out.Flush();
        using var serverCertificate =
            new X509Certificate2(Path.Combine(certificatesPath, "ca1/server.p12"), "password");
        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = serverCertificate,
        };
        using var serverCommunicator = createServer(serverOptions, helper);

        var clientOptions = new SslClientAuthenticationOptions
        {
            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) => false
        };
        using var clientCommunicator = createClient(clientOptions);

        var obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch (Ice.SecurityException)
        {
            // Expected
        }
        Console.Out.WriteLine("ok");
    }

    private static void clientRejectServerUsingDefaultValidationCallback(TestHelper helper, string certificatesPath)
    {
        Console.Out.Write("client rejects server certificate using default validation callback... ");
        Console.Out.Flush();
        using var serverCertificate =
            new X509Certificate2(Path.Combine(certificatesPath, "ca1/server.p12"), "password");
        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = serverCertificate,
        };
        using var serverCommunicator = createServer(serverOptions, helper);

        using var clientCommunicator = createClient(new SslClientAuthenticationOptions { });

        var obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch (Ice.SecurityException)
        {
            // Expected
        }
        Console.Out.WriteLine("ok");
    }
    private static void serverValidatesClientUsingValidationCallback(TestHelper helper, string certificatesPath)
    {
        Console.Out.Write("server validates client certificate using validation callback... ");
        Console.Out.Flush();
        using var serverCertificate =
            new X509Certificate2(Path.Combine(certificatesPath, "ca1/server.p12"), "password");
        using var clientCertificate =
            new X509Certificate2(Path.Combine(certificatesPath, "ca1/client.p12"), "password");
        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = serverCertificate,
            ClientCertificateRequired = true,
            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                certificate.Subject == clientCertificate.Subject,
        };
        using var serverCommunicator = createServer(serverOptions, helper);

        var clientOptions = new SslClientAuthenticationOptions
        {
            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                certificate.Subject == serverCertificate.Subject,
            ClientCertificates =
            [
                clientCertificate
            ]
        };
        using var clientCommunicator = createClient(clientOptions);

        var obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
        obj.ice_ping();
        Console.WriteLine("ok");
    }

    private static void serverRejectsClientUsingValidationCallback(TestHelper helper, string certificatesPath)
    {
        Console.Out.Write("server rejects client certificate using validation callback... ");
        Console.Out.Flush();
        using var serverCertificate = new X509Certificate2(Path.Combine(certificatesPath, "ca1/server.p12"), "password");
        using var clientCertificate = new X509Certificate2(Path.Combine(certificatesPath, "ca1/client.p12"), "password");
        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = serverCertificate,
            ClientCertificateRequired = true,
            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) => false,
        };
        using var serverCommunicator = createServer(serverOptions, helper);

        var clientOptions = new SslClientAuthenticationOptions
        {
            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                certificate.Subject == serverCertificate.Subject,
            ClientCertificates =
            [
                clientCertificate
            ]
        };
        using var clientCommunicator = createClient(clientOptions);

        var obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
        try
        {
            obj.ice_ping();
            test(false);
        }
        catch (Ice.ConnectionLostException)
        {
        }
        Console.WriteLine("ok");
    }

    private static void serverRejectsClientUsingDefaultValidationCallback(TestHelper helper, string certificatesPath)
    {
        Console.Out.Write("server rejects client certificate using default validation callback... ");
        Console.Out.Flush();
        using var serverCertificate = new X509Certificate2(Path.Combine(certificatesPath, "ca1/server.p12"), "password");
        using var clientCertificate = new X509Certificate2(Path.Combine(certificatesPath, "ca1/client.p12"), "password");
        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = serverCertificate,
            ClientCertificateRequired = true,
        };
        using var serverCommunicator = createServer(serverOptions, helper);

        var clientOptions = new SslClientAuthenticationOptions
        {
            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                certificate.Subject == serverCertificate.Subject,
            ClientCertificates =
            [
                clientCertificate
            ]
        };
        using var clientCommunicator = createClient(clientOptions);

        var obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));

        try
        {
            obj.ice_ping();
            test(false);
        }
        catch (Ice.ConnectionLostException)
        {
        }
        Console.WriteLine("ok");
    }

    private sealed class ServerState : IDisposable
    {
        public X509Certificate2 Certificate { get; private set; }

        public ServerState(string certificatePath) => Certificate = new X509Certificate2(certificatePath, "password");

        public void Dispose() => Certificate?.Dispose();

        public void reloadCertificate(string certificatePath)
        {
            Certificate?.Dispose();
            Certificate = new X509Certificate2(certificatePath, "password");
        }
    }

    private static void serverHotCertificateReload(TestHelper helper, string certificatesPath)
    {
        Console.Out.Write("server hot certificate reload... ");
        Console.Out.Flush();

        var trustedRootCertificatesCA1 = new X509Certificate2(Path.Combine(certificatesPath, "ca1/ca1_cert.pem"));
        var trustedRootCertificatesCA2 = new X509Certificate2(Path.Combine(certificatesPath, "ca2/ca2_cert.pem"));

        var serverState = new ServerState(Path.Combine(certificatesPath, "ca1/server.p12"));

        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificateSelectionCallback = (sender, hostName) => serverState.Certificate
        };
        using var serverCommunicator = createServer(serverOptions, helper);

        {
            var clientOptions = new SslClientAuthenticationOptions
            {
                RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                    certificate.Issuer == trustedRootCertificatesCA1.Subject
            };
            using var clientCommunicator = createClient(clientOptions);

            var obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
            obj.ice_ping();
        }

        {
            // CA2 is not accepted with the initial configuration
            var clientOptions = new SslClientAuthenticationOptions
            {
                RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                    certificate.Issuer == trustedRootCertificatesCA2.Subject
            };
            using var clientCommunicator = createClient(clientOptions);

            var obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
            try
            {
                obj.ice_ping();
                test(false);
            }
            catch (Ice.SecurityException)
            {
                // Expected
            }
        }

        serverState.reloadCertificate(Path.Combine(certificatesPath, "ca2/server.p12"));

        {
            // CA2 is accepted with the new configuration
            var clientOptions = new SslClientAuthenticationOptions
            {
                RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                    certificate.Issuer == trustedRootCertificatesCA2.Subject
            };
            using var clientCommunicator = createClient(clientOptions);

            var obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
            obj.ice_ping();
        }

        {
            // CA1 is not accepted with the initial configuration
            var clientOptions = new SslClientAuthenticationOptions
            {
                RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                    certificate.Issuer == trustedRootCertificatesCA1.Subject
            };
            using var clientCommunicator = createClient(clientOptions);

            var obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
            try
            {
                obj.ice_ping();
                test(false);
            }
            catch (Ice.SecurityException)
            {
                // Expected
            }
        }

        Console.WriteLine("ok");
    }

    public static void allTests(TestHelper helper, string certificatesPath)
    {
        clientValidatesServerUsingValidationCallback(helper, certificatesPath);
        clientValidatesServerUsingSystemTrustedRootCertificates(helper, certificatesPath);
        clientRejectServerUsingValidationCallback(helper, certificatesPath);
        clientRejectServerUsingDefaultValidationCallback(helper, certificatesPath);

        serverValidatesClientUsingValidationCallback(helper, certificatesPath);
        serverRejectsClientUsingValidationCallback(helper, certificatesPath);
        serverRejectsClientUsingDefaultValidationCallback(helper, certificatesPath);

        serverHotCertificateReload(helper, certificatesPath);
    }
}
