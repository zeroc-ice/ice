// Copyright (c) ZeroC, Inc.

using System.Net.Security;
using System.Security.Cryptography.X509Certificates;
using Test;

namespace Ice.SSL.configuration;

public static class PlatformTests
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new System.Exception();
        }
    }

    private static Ice.Communicator createServer(
        SslServerAuthenticationOptions serverAuthenticationOptions,
        TestHelper helper)
    {
        var communicator = new Ice.Communicator();
        ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints(
            "ServerAdapter",
            helper.getTestEndpoint(10, "ssl"),
            serverAuthenticationOptions);
        adapter.add(new ServerI(communicator), Ice.Util.stringToIdentity("server"));
        adapter.activate();
        return communicator;
    }

    private static Ice.Communicator createClient(SslClientAuthenticationOptions clientAuthenticationOptions)
    {
        var communicator = new Ice.Communicator(new Ice.InitializationData
        {
            clientAuthenticationOptions = clientAuthenticationOptions
        });
        return communicator;
    }

    private static void clientValidatesServerUsingValidationCallback(TestHelper helper, string certificatesPath)
    {
        Console.Out.Write("client validates server certificate using validation callback... ");
        Console.Out.Flush();
        using X509Certificate2 serverCertificate =
            X509CertificateLoader.LoadPkcs12FromFile(Path.Combine(certificatesPath, "ca1", "server.p12"), "password");
        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = serverCertificate,
        };
        using Ice.Communicator serverCommunicator = createServer(serverOptions, helper);

        var clientOptions = new SslClientAuthenticationOptions
        {
            RemoteCertificateValidationCallback =
                (sender, certificate, chain, sslPolicyErrors) => certificate.Subject == serverCertificate.Subject
        };
        using Ice.Communicator clientCommunicator = createClient(clientOptions);

        ServerPrx obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
        obj.ice_ping();
        Console.Out.WriteLine("ok");
    }

    private static void clientRejectServerUsingValidationCallback(TestHelper helper, string certificatesPath)
    {
        Console.Out.Write("client rejects server certificate using validation callback... ");
        Console.Out.Flush();
        using X509Certificate2 serverCertificate =
            X509CertificateLoader.LoadPkcs12FromFile(Path.Combine(certificatesPath, "ca1", "server.p12"), "password");
        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = serverCertificate,
        };
        using Ice.Communicator serverCommunicator = createServer(serverOptions, helper);

        var clientOptions = new SslClientAuthenticationOptions
        {
            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) => false
        };
        using Ice.Communicator clientCommunicator = createClient(clientOptions);

        ServerPrx obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
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
        using X509Certificate2 serverCertificate =
            X509CertificateLoader.LoadPkcs12FromFile(Path.Combine(certificatesPath, "ca1", "server.p12"), "password");
        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = serverCertificate,
        };
        using Ice.Communicator serverCommunicator = createServer(serverOptions, helper);

        using Ice.Communicator clientCommunicator = createClient(new SslClientAuthenticationOptions { });

        ServerPrx obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
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
        using X509Certificate2 serverCertificate =
            X509CertificateLoader.LoadPkcs12FromFile(Path.Combine(certificatesPath, "ca1", "server.p12"), "password");
        using X509Certificate2 clientCertificate =
            X509CertificateLoader.LoadPkcs12FromFile(Path.Combine(certificatesPath, "ca1", "client.p12"), "password");
        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = serverCertificate,
            ClientCertificateRequired = true,
            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                certificate.Subject == clientCertificate.Subject,
        };
        using Ice.Communicator serverCommunicator = createServer(serverOptions, helper);

        var clientOptions = new SslClientAuthenticationOptions
        {
            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                certificate.Subject == serverCertificate.Subject,
            ClientCertificates =
            [
                clientCertificate
            ]
        };
        using Ice.Communicator clientCommunicator = createClient(clientOptions);

        ServerPrx obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
        obj.ice_ping();
        Console.WriteLine("ok");
    }

    private static void serverRejectsClientUsingValidationCallback(TestHelper helper, string certificatesPath)
    {
        Console.Out.Write("server rejects client certificate using validation callback... ");
        Console.Out.Flush();
        using X509Certificate2 serverCertificate =
            X509CertificateLoader.LoadPkcs12FromFile(Path.Combine(certificatesPath, "ca1", "server.p12"), "password");
        using X509Certificate2 clientCertificate =
            X509CertificateLoader.LoadPkcs12FromFile(Path.Combine(certificatesPath, "ca1", "client.p12"), "password");
        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = serverCertificate,
            ClientCertificateRequired = true,
            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) => false,
        };
        using Ice.Communicator serverCommunicator = createServer(serverOptions, helper);

        var clientOptions = new SslClientAuthenticationOptions
        {
            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                certificate.Subject == serverCertificate.Subject,
            ClientCertificates =
            [
                clientCertificate
            ]
        };
        using Ice.Communicator clientCommunicator = createClient(clientOptions);

        ServerPrx obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
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
        using X509Certificate2 serverCertificate =
            X509CertificateLoader.LoadPkcs12FromFile(Path.Combine(certificatesPath, "ca1", "server.p12"), "password");
        using X509Certificate2 clientCertificate =
            X509CertificateLoader.LoadPkcs12FromFile(Path.Combine(certificatesPath, "ca1", "client.p12"), "password");
        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = serverCertificate,
            ClientCertificateRequired = true,
        };
        using Ice.Communicator serverCommunicator = createServer(serverOptions, helper);

        var clientOptions = new SslClientAuthenticationOptions
        {
            RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                certificate.Subject == serverCertificate.Subject,
            ClientCertificates =
            [
                clientCertificate
            ]
        };
        using Ice.Communicator clientCommunicator = createClient(clientOptions);

        ServerPrx obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));

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

        public ServerState(string certificatePath) =>
            Certificate = X509CertificateLoader.LoadPkcs12FromFile(certificatePath, "password");

        public void Dispose() => Certificate?.Dispose();

        public void reloadCertificate(string certificatePath)
        {
            Certificate?.Dispose();
            Certificate = X509CertificateLoader.LoadPkcs12FromFile(certificatePath, "password");
        }
    }

    private static void serverHotCertificateReload(TestHelper helper, string certificatesPath)
    {
        Console.Out.Write("server hot certificate reload... ");
        Console.Out.Flush();

        using X509Certificate2 trustedRootCertificatesCA1 =
            X509CertificateLoader.LoadCertificateFromFile(Path.Combine(certificatesPath, "ca1", "ca1_cert.pem"));
        using X509Certificate2 trustedRootCertificatesCA2 =
            X509CertificateLoader.LoadCertificateFromFile(Path.Combine(certificatesPath, "ca2", "ca2_cert.pem"));

        using var serverState = new ServerState(Path.Combine(certificatesPath, "ca1", "server.p12"));

        var serverOptions = new SslServerAuthenticationOptions
        {
            ServerCertificateSelectionCallback = (sender, hostName) => serverState.Certificate
        };

        using Ice.Communicator serverCommunicator = createServer(serverOptions, helper);
        {
            var clientOptions = new SslClientAuthenticationOptions
            {
                RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                    certificate.Issuer == trustedRootCertificatesCA1.Subject
            };
            using Ice.Communicator clientCommunicator = createClient(clientOptions);

            ServerPrx obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
            obj.ice_ping();
        }

        {
            // CA2 is not accepted with the initial configuration
            var clientOptions = new SslClientAuthenticationOptions
            {
                RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                    certificate.Issuer == trustedRootCertificatesCA2.Subject
            };
            using Ice.Communicator clientCommunicator = createClient(clientOptions);

            ServerPrx obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
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

        serverState.reloadCertificate(Path.Combine(certificatesPath, "ca2", "server.p12"));
        {
            // CA2 is accepted with the new configuration
            var clientOptions = new SslClientAuthenticationOptions
            {
                RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                    certificate.Issuer == trustedRootCertificatesCA2.Subject
            };
            using Ice.Communicator clientCommunicator = createClient(clientOptions);

            ServerPrx obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
            obj.ice_ping();
        }

        {
            // CA1 is not accepted with the initial configuration
            var clientOptions = new SslClientAuthenticationOptions
            {
                RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                    certificate.Issuer == trustedRootCertificatesCA1.Subject
            };
            using Ice.Communicator clientCommunicator = createClient(clientOptions);

            ServerPrx obj = ServerPrxHelper.createProxy(clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
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
        clientRejectServerUsingValidationCallback(helper, certificatesPath);
        clientRejectServerUsingDefaultValidationCallback(helper, certificatesPath);

        serverValidatesClientUsingValidationCallback(helper, certificatesPath);
        serverRejectsClientUsingValidationCallback(helper, certificatesPath);
        serverRejectsClientUsingDefaultValidationCallback(helper, certificatesPath);

        serverHotCertificateReload(helper, certificatesPath);
    }
}
