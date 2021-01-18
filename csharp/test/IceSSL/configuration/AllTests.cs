// Copyright (c) ZeroC, Inc. All rights reserved.

// NOTE: This test is not interoperable with other language mappings.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Ice;
using ZeroC.Test;

namespace ZeroC.IceSSL.Test.Configuration
{
    public class Blobject : IObject
    {
        public ValueTask<OutgoingResponseFrame> DispatchAsync(
            IncomingRequestFrame request,
            Current current,
            CancellationToken cancel)
        {
            if (current.Operation.Equals("ice_ping"))
            {
                return new(OutgoingResponseFrame.WithVoidReturnValue(current));
            }
            else
            {
                throw new OperationNotExistException();
            }
        }
    }

    public static class AllTests
    {
        private static X509Certificate2 CreateCertificate(string certPEM) =>
            new(System.Text.Encoding.ASCII.GetBytes(certPEM));

        private static Dictionary<string, string>
        CreateProperties(Dictionary<string, string> defaultProperties, string? cert = null, string? ca = null)
        {
            var properties = new Dictionary<string, string>(defaultProperties);

            properties["Ice.AcceptNonSecure"] = "Never";

            if (defaultProperties.TryGetValue("IceSSL.DefaultDir", out string? value))
            {
                properties["IceSSL.DefaultDir"] = value;
            }

            if (defaultProperties.TryGetValue("Test.Host", out value))
            {
                properties["Test.Host"] = value;
            }

            properties["Ice.RetryIntervals"] = "-1";

            if (cert != null)
            {
                properties["IceSSL.CertFile"] = $"{cert}.p12";
            }

            if (ca != null)
            {
                properties["IceSSL.CAs"] = $"{ca}.pem";
            }
            properties["IceSSL.Password"] = "password";

            return properties;
        }

        private static IServerFactoryPrx
        CreateServerFactoryPrx(string factoryRef, Communicator communicator) =>
            IServerFactoryPrx.Parse(factoryRef, communicator).Clone(preferNonSecure: NonSecure.Always);

        public static async Task RunAsync(TestHelper helper, string testDir)
        {
            Communicator communicator = helper.Communicator;

            string factoryRef = helper.GetTestProxy("factory", 0, "tcp");
            var factory = CreateServerFactoryPrx(factoryRef, communicator);

            string host = TestHelper.GetTestHost(communicator.GetProperties());
            string defaultDir = $"{testDir}/../certs";
            Dictionary<string, string> defaultProperties = communicator.GetProperties();
            defaultProperties["IceSSL.DefaultDir"] = defaultDir;
            defaultProperties.Remove("IceSSL.CertFile");
            defaultProperties.Remove("IceSSL.CAs");

            // Load the CA certificates. We could use the IceSSL.ImportCert property, but it would be nice to remove
            // the CA certificates when the test finishes, so this test manually installs the certificates in the
            // LocalMachine:AuthRoot store.
            //
            // Note that the client and server are assumed to run on the same machine, so the certificates installed
            // by the client are also available to the server.
            string caCert1File = defaultDir + "/cacert1.pem";
            string caCert2File = defaultDir + "/cacert2.pem";
            var caCert1 = new X509Certificate2(caCert1File);
            var caCert2 = new X509Certificate2(caCert2File);

            TestHelper.Assert(Enumerable.SequenceEqual(CreateCertificate(File.ReadAllText(caCert1File)).RawData,
                caCert1.RawData));
            TestHelper.Assert(Enumerable.SequenceEqual(CreateCertificate(File.ReadAllText(caCert2File)).RawData,
                caCert2.RawData));

            var store = new X509Store(StoreName.AuthRoot, StoreLocation.LocalMachine);
            bool isAdministrator = false;
            if (OperatingSystem.IsWindows())
            {
                try
                {
                    store.Open(OpenFlags.ReadWrite);
                    isAdministrator = true;
                }
                catch (CryptographicException)
                {
                    store.Open(OpenFlags.ReadOnly);
                    Console.Out.WriteLine(
                        "warning: some test requires administrator privileges, run as Administrator to run all the tests.");
                }
            }

            Dictionary<string, string> clientProperties;
            Dictionary<string, string> serverProperties;
            try
            {
                string[] args = Array.Empty<string>();

                Console.Out.Write("testing initialization... ");
                Console.Out.Flush();

                {
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    IServerPrx server = fact.CreateServer(serverProperties, true);

                    try
                    {
                        server.IcePing();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }

                {
                    // Supply our own certificate.
                    clientProperties = CreateProperties(defaultProperties);
                    clientProperties["IceSSL.CAs"] = caCert1File;
                    await using var comm = new Communicator(ref args, clientProperties,
                        tlsClientOptions: new TlsClientOptions()
                        {
                            ClientCertificates = new X509Certificate2Collection
                            {
                                new X509Certificate2(defaultDir + "/c_rsa_ca1.p12", "password")
                            }
                        });
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        server.IcePing();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }

                {
                    // Supply our own certificate.
                    clientProperties = CreateProperties(defaultProperties);
                    clientProperties["IceSSL.CAs"] = caCert1File;

                    var cert0 = new X509Certificate2(Path.Combine(defaultDir, "c_rsa_ca1.p12"), "password");
                    var cert1 = new X509Certificate2(Path.Combine(defaultDir, "c_rsa_ca2.p12"), "password");

                    await using var comm = new Communicator(ref args, clientProperties,
                        tlsClientOptions: new TlsClientOptions()
                        {
                            ClientCertificates = new X509Certificate2Collection
                            {
                                cert0,
                                cert1
                            },
                            ClientCertificateSelectionCallback =
                                (sender, targetHost, certs, remoteCertificate, acceptableIssuers) =>
                                {
                                    TestHelper.Assert(certs.Count == 2);
                                    TestHelper.Assert(certs[0] == cert0);
                                    TestHelper.Assert(certs[1] == cert1);
                                    return certs[0];
                                }
                        });
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        server.IcePing();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }

                {
                    // Supply our own CA certificate.
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");
                    await using var comm = new Communicator(ref args, clientProperties,
                        tlsClientOptions: new TlsClientOptions()
                        {
                            ServerCertificateCertificateAuthorities = new X509Certificate2Collection()
                            {
                                new X509Certificate2(Path.Combine(defaultDir, "cacert1.pem"))
                            }
                        });

                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        server.IcePing();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }

                {
                    // Initialization using TlsClientOptions and TlsServerOptions options
                    await using var clientCommunicator = new Communicator(ref args, CreateProperties(defaultProperties),
                        tlsClientOptions: new TlsClientOptions()
                        {
                            ClientCertificates = new X509Certificate2Collection()
                            {
                                new X509Certificate2(Path.Combine(defaultDir, "c_rsa_ca1.p12"), "password")
                            },
                            ServerCertificateCertificateAuthorities = new X509Certificate2Collection()
                            {
                                new X509Certificate2(Path.Combine(defaultDir, "cacert1.pem"))
                            }
                        });

                    await using var serverCommunicator = new Communicator(ref args, CreateProperties(defaultProperties),
                        tlsServerOptions: new TlsServerOptions()
                        {
                            ServerCertificate = new X509Certificate2(
                                Path.Combine(defaultDir, "s_rsa_ca1.p12"), "password"),
                            RequireClientCertificate = true,
                            ClientCertificateCertificateAuthorities = new X509Certificate2Collection()
                            {
                                new X509Certificate2(Path.Combine(defaultDir, "cacert1.pem"))
                            }
                        });

                    bool ice1 = helper.Protocol == Protocol.Ice1;
                    ObjectAdapter adapter = serverCommunicator.CreateObjectAdapterWithEndpoints(
                            "MyAdapter", ice1 ? $"ssl -h {host}" : $"ice+tcp://{host}:0");
                    IObjectPrx? prx = adapter.AddWithUUID(new Blobject(), IObjectPrx.Factory);
                    await adapter.ActivateAsync();
                    prx = IObjectPrx.Parse(prx.ToString()!, clientCommunicator);
                    prx.IcePing();
                }
                {
                    // Initialization using TlsClientOptions and TlsServerOptions options, validate using
                    // validation callbacks
                    var clientCertificate =
                        new X509Certificate2(Path.Combine(defaultDir, "c_rsa_ca1.p12"), "password");
                    var serverCertificate =
                        new X509Certificate2(Path.Combine(defaultDir, "s_rsa_ca1.p12"), "password");

                    bool clientCertificateValidationCallbackCalled = false;
                    bool serverCertificateValidationCallbackCalled = false;

                    clientProperties = CreateProperties(defaultProperties);
                    await using var clientCommunicator = new Communicator(ref args, clientProperties,
                        tlsClientOptions: new TlsClientOptions()
                        {
                            ClientCertificateSelectionCallback =
                                (sender, targetHost, certs, remoteCertificate, acceptableIssuers) =>
                                    clientCertificate,
                            ServerCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                            {
                                serverCertificateValidationCallbackCalled = true;
                                return certificate!.GetCertHashString() == serverCertificate.GetCertHashString();
                            }
                        });

                    serverProperties = CreateProperties(defaultProperties);
                    await using var serverCommunicator = new Communicator(ref args, serverProperties,
                        tlsServerOptions: new TlsServerOptions()
                        {
                            ServerCertificate = serverCertificate,
                            RequireClientCertificate = true,
                            ClientCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                            {
                                clientCertificateValidationCallbackCalled = true;
                                return certificate!.GetCertHashString() == clientCertificate.GetCertHashString();
                            }
                        });

                    bool ice1 = helper.Protocol == Protocol.Ice1;
                    ObjectAdapter adapter = serverCommunicator.CreateObjectAdapterWithEndpoints(
                        "MyAdapter", ice1 ? $"ssl -h {host}" : $"ice+tcp://{host}:0");
                    IObjectPrx? prx = adapter.AddWithUUID(new Blobject(), IObjectPrx.Factory);
                    await adapter.ActivateAsync();
                    prx = IObjectPrx.Parse(prx.ToString()!, clientCommunicator);
                    prx.IcePing();
                    TestHelper.Assert(clientCertificateValidationCallbackCalled);
                    TestHelper.Assert(serverCertificateValidationCallbackCalled);
                }

                {
                    try
                    {
                        // Setting ServerCertificateCertificateAuthorities is incompatible with ServerCertificateValidationCallback
                        _ = new TlsClientOptions()
                        {
#pragma warning disable CA5359 // Do Not Disable Certificate Validation
                            ServerCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) => true,
#pragma warning restore CA5359 // Do Not Disable Certificate Validation
                            ServerCertificateCertificateAuthorities = new X509Certificate2Collection()
                        };
                        TestHelper.Assert(false);
                    }
                    catch (ArgumentException)
                    {
                    }

                    try
                    {
                        // Setting ServerCertificateValidationCallback is incompatible with ServerCertificateCertificateAuthorities
                        _ = new TlsClientOptions()
                        {
                            ServerCertificateCertificateAuthorities = new X509Certificate2Collection(),
#pragma warning disable CA5359 // Do Not Disable Certificate Validation
                            ServerCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) => true
#pragma warning restore CA5359 // Do Not Disable Certificate Validation
                        };
                        TestHelper.Assert(false);
                    }
                    catch (ArgumentException)
                    {
                    }

                    try
                    {
                        // Setting ClientCertificateCertificateAuthorities is incompatible with ClientCertificateValidationCallback
                        _ = new TlsServerOptions()
                        {
#pragma warning disable CA5359 // Do Not Disable Certificate Validation
                            ClientCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) => true,
#pragma warning restore CA5359 // Do Not Disable Certificate Validation
                            ClientCertificateCertificateAuthorities = new X509Certificate2Collection()
                        };
                        TestHelper.Assert(false);
                    }
                    catch (ArgumentException)
                    {
                    }

                    try
                    {
                        // Setting ClientCertificateValidationCallback is incompatible with ClientCertificateCertificateAuthorities
                        _ = new TlsServerOptions()
                        {
                            ClientCertificateCertificateAuthorities = new X509Certificate2Collection(),
#pragma warning disable CA5359 // Do Not Disable Certificate Validation
                            ClientCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) => true
#pragma warning restore CA5359 // Do Not Disable Certificate Validation
                        };
                        TestHelper.Assert(false);
                    }
                    catch (ArgumentException)
                    {
                    }
                }
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing certificate verification... ");
                Console.Out.Flush();
                {
                    // The server doesn't request a certificate, but it still verifies the server's.
                    clientProperties = CreateProperties(defaultProperties, ca: "cacert1");
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                    IServerPrx server = fact.CreateServer(serverProperties, false);
                    try
                    {
                        server.NoCert();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }

                {
                    // This should fail because the client does not supply a certificate and server request it.
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        server.IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (ConnectionLostException)
                    {
                        // Expected.
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }
                {
                    // Test client has a certificate and server request it.
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        var clientCert = new X509Certificate2(defaultDir + "/c_rsa_ca1.p12", "password");
                        server.CheckCert(clientCert.Subject, clientCert.Issuer);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }
                {
                    // This should fail because the client doesn't trust the server's CA.
                    clientProperties = CreateProperties(defaultProperties);
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                    IServerPrx server = fact.CreateServer(serverProperties, false);
                    try
                    {
                        server.IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (TransportException)
                    {
                        // Expected.
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }
                {
                    // This should fail because the server doesn't trust the client's CA.
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca2");
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        server.IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (TransportException)
                    {
                        // Expected.
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }
                {
                    // This should succeed because the self signed certificate used by the server is trusted.
                    clientProperties = CreateProperties(defaultProperties, ca: "cacert2");
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "cacert2");
                    IServerPrx server = fact.CreateServer(serverProperties, false);
                    try
                    {
                        server.IcePing();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }
                {
                    // This should fail because the self signed certificate used by the server is not trusted.
                    clientProperties = CreateProperties(defaultProperties);
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "cacert2");
                    IServerPrx server = fact.CreateServer(serverProperties, false);
                    try
                    {
                        server.IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (TransportException)
                    {
                        // Expected.
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }

                {
                    // Test Hostname verification only when Test.Host is 127.0.0.1 as that is the IP address used
                    // in the test certificates.
                    if (host.Equals("127.0.0.1"))
                    {
                        // Test using localhost as target host.
                        var props = new Dictionary<string, string>(defaultProperties)
                        {
                            ["Test.Host"] = "localhost",
                            ["Ice.ServerName"] = "localhost",
                        };

                        // This must succeed, the target host matches the certificate DNS altName.
                        {
                            clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                            await using var comm = new Communicator(ref args, clientProperties);

                            var fact = CreateServerFactoryPrx(factoryRef, comm);
                            serverProperties = CreateProperties(props, "s_rsa_ca1_cn1", "cacert1");
                            IServerPrx server = fact.CreateServer(serverProperties, true)!;

                            TestHelper.Assert(server.Endpoints.All(endpoint => endpoint.Host == "localhost"));

                            try
                            {
                                server.IcePing();
                            }
                            catch (Exception ex)
                            {
                                Console.WriteLine(ex.ToString());
                                TestHelper.Assert(false);
                            }
                            fact.DestroyServer(server);
                        }

                        // This must fail, the target host does not match the certificate DNS altName.
                        {
                            clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                            await using var comm = new Communicator(ref args, clientProperties);

                            var fact = CreateServerFactoryPrx(factoryRef, comm);
                            serverProperties = CreateProperties(props, "s_rsa_ca1_cn2", "cacert1");
                            IServerPrx server = fact.CreateServer(serverProperties, true);
                            try
                            {
                                server.IcePing();
                                TestHelper.Assert(false);
                            }
                            catch (TransportException)
                            {
                                // Expected
                            }
                            fact.DestroyServer(server);
                        }

                        // This must succeed, the target host matches the certificate Common Name and the certificate
                        // does not include a DNS altName.
                        {
                            clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                            await using var comm = new Communicator(ref args, clientProperties);

                            var fact = CreateServerFactoryPrx(factoryRef, comm);
                            serverProperties = CreateProperties(props, "s_rsa_ca1_cn3", "cacert1");
                            IServerPrx server = fact.CreateServer(serverProperties, true);
                            try
                            {
                                server.IcePing();
                            }
                            catch
                            {
                                // macOS >= Catalina requires a DNS altName. DNS name as the Common Name is not trusted
                                TestHelper.Assert(OperatingSystem.IsMacOS());
                            }
                            fact.DestroyServer(server);
                        }

                        // This must fail, the target host does not match the certificate Common Name and the
                        // certificate does not include a DNS altName.
                        {
                            clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                            await using var comm = new Communicator(ref args, clientProperties);

                            var fact = CreateServerFactoryPrx(factoryRef, comm);
                            serverProperties = CreateProperties(props, "s_rsa_ca1_cn4", "cacert1");
                            IServerPrx server = fact.CreateServer(serverProperties, true);
                            try
                            {
                                server.IcePing();
                                TestHelper.Assert(false);
                            }
                            catch (TransportException)
                            {
                                // Expected
                            }
                            fact.DestroyServer(server);
                        }

                        // This must fail, the target host matches the certificate Common Name and the certificate has
                        // a DNS altName that does not matches the target host
                        {
                            clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                            await using var comm = new Communicator(ref args, clientProperties);

                            var fact = CreateServerFactoryPrx(factoryRef, comm);
                            serverProperties = CreateProperties(props, "s_rsa_ca1_cn5", "cacert1");
                            IServerPrx server = fact.CreateServer(serverProperties, true);
                            try
                            {
                                server.IcePing();
                                TestHelper.Assert(false);
                            }
                            catch (TransportException)
                            {
                                // Expected
                            }
                            fact.DestroyServer(server);
                        }

                        // Test using 127.0.0.1 as target host

                        // TODO enable or remove
                        // Disabled for compatibility with older Windows versions.
                        /* //
                        // Target host matches the certificate IP altName
                        //
                        {
                            clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                            comm = new Communicator(ref args, initData);

                            fact = IServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                            TestHelper.Assert(fact != null);
                            serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1_cn6", "cacert1");
                            server = fact.CreateServer(d);
                            try
                            {
                                server.IcePing();
                            }
                            catch
                            {
                                TestHelper.Assert(false);
                            }
                            fact.DestroyServer(server);
                            comm.Destroy();
                        }
                        //
                        // Target host does not match the certificate IP altName
                        //
                        {
                            clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                            comm = new Communicator(ref args, initData);

                            fact = IServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                            TestHelper.Assert(fact != null);
                            serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1_cn7", "cacert1");
                            server = fact.CreateServer(d);
                            try
                            {
                                server.IcePing();
                                TestHelper.Assert(false);
                            }
                            catch(Ice.SecurityException)
                            {
                                // Expected
                            }
                            fact.DestroyServer(server);
                            comm.Destroy();
                        }*/

                        // Target host is an IP address that matches the CN and the certificate doesn't include an IP
                        // altName.
                        {
                            clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                            await using var comm = new Communicator(ref args, clientProperties);

                            var fact = CreateServerFactoryPrx(factoryRef, comm);
                            serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1_cn8", "cacert1");
                            IServerPrx server = fact.CreateServer(serverProperties, true);
                            try
                            {
                                server.IcePing();
                            }
                            catch (TransportException ex)
                            {
                                // macOS catalina does not check the certificate common name
                                if (!OperatingSystem.IsMacOS())
                                {
                                    Console.WriteLine(ex.ToString());
                                    TestHelper.Assert(false);
                                }
                            }
                            fact.DestroyServer(server);
                        }

                        // Target host does not match the certificate DNS altName, connection should fail.
                        {
                            clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                            await using var comm = new Communicator(ref args, clientProperties);

                            var fact = CreateServerFactoryPrx(factoryRef, comm);
                            serverProperties = CreateProperties(props, "s_rsa_ca1_cn2", "cacert1");
                            IServerPrx server = fact.CreateServer(serverProperties, true);
                            try
                            {
                                server.IcePing();
                                TestHelper.Assert(false);
                            }
                            catch (TransportException)
                            {
                            }
                            catch (Exception ex)
                            {
                                Console.WriteLine(ex.ToString());
                                TestHelper.Assert(false);
                            }
                            fact.DestroyServer(server);
                        }
                    }
                }
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing certificate selection callback... ");
                Console.Out.Flush();
                {
                    var myCerts = new X509Certificate2Collection();
                    myCerts.Import(defaultDir + "/c_rsa_ca1.p12", "password", X509KeyStorageFlags.DefaultKeySet);
                    bool called = false;
                    await using var comm = new Communicator(ref args, clientProperties,
                        tlsClientOptions: new TlsClientOptions()
                        {
                            ClientCertificateSelectionCallback =
                                (sender, targetHost, certs, remoteCertificate, acceptableIssuers) =>
                                {
                                    called = true;
                                    return myCerts[0];
                                }
                        });

                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    TestHelper.Assert(server != null);
                    try
                    {
                        server.IcePing();
                        var connection = server.GetCachedConnection() as TcpConnection;
                        TestHelper.Assert(connection != null);
                        TestHelper.Assert(connection.LocalCertificate == myCerts[0]);
                        TestHelper.Assert(called);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing certificate chains... ");
                Console.Out.Flush();
                {
                    var certStore = new X509Store("My", StoreLocation.CurrentUser);
                    certStore.Open(OpenFlags.ReadWrite);
                    var certs = new X509Certificate2Collection();
                    X509KeyStorageFlags storageFlags = X509KeyStorageFlags.DefaultKeySet;
                    if (OperatingSystem.IsMacOS())
                    {
                        // On macOS, we need to mark the key exportable because the addition of the key to the cert
                        // store requires to move the key from on keychain to another (which requires the Exportable
                        // flag... see https://github.com/dotnet/corefx/issues/25631)
                        storageFlags |= X509KeyStorageFlags.Exportable;
                    }
                    certs.Import(defaultDir + "/s_rsa_cai2.p12", "password", storageFlags);
                    foreach (X509Certificate2 cert in certs)
                    {
                        certStore.Add(cert);
                    }
                    try
                    {
                        {
                            clientProperties = CreateProperties(defaultProperties);
                            await using var comm = new Communicator(clientProperties);

                            var fact = CreateServerFactoryPrx(factoryRef, comm);

                            // The client can't verify the server certificate it should fail.
                            serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                            IServerPrx server = fact.CreateServer(serverProperties, false);
                            try
                            {
                                server.IcePing();
                                TestHelper.Assert(false);
                            }
                            catch (TransportException)
                            {
                            }
                            catch (Exception ex)
                            {
                                Console.WriteLine(ex.ToString());
                                TestHelper.Assert(false);
                            }
                            fact.DestroyServer(server);

                            // Setting the CA for the server shouldn't change anything, it shouldn't modify the cert
                            // chain sent to the client.
                            serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                            server = fact.CreateServer(serverProperties, false);
                            try
                            {
                                server.IcePing();
                                TestHelper.Assert(false);
                            }
                            catch (TransportException)
                            {
                            }
                            catch (Exception ex)
                            {
                                Console.WriteLine(ex.ToString());
                                TestHelper.Assert(false);
                            }
                            fact.DestroyServer(server);

                            // The client can't verify the server certificate should fail.
                            serverProperties = CreateProperties(defaultProperties, "s_rsa_wroot_ca1");
                            server = fact.CreateServer(serverProperties, false);
                            try
                            {
                                server.IcePing();
                                TestHelper.Assert(false);
                            }
                            catch (TransportException)
                            {
                            }
                            catch (Exception ex)
                            {
                                Console.WriteLine(ex.ToString());
                                TestHelper.Assert(false);
                            }
                            fact.DestroyServer(server);
                        }

                        {
                            // Now the client verifies the server certificate
                            clientProperties = CreateProperties(defaultProperties, ca: "cacert1");
                            await using var comm = new Communicator(clientProperties);

                            var fact = CreateServerFactoryPrx(factoryRef, comm);
                            serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                            IServerPrx server = fact.CreateServer(serverProperties, false);
                            try
                            {
                                var tcpConnection = (TcpConnection)await server.GetConnectionAsync();
                                TestHelper.Assert(tcpConnection.IsEncrypted);
                            }
                            catch (Exception ex)
                            {
                                Console.WriteLine(ex.ToString());
                                TestHelper.Assert(false);
                            }
                            fact.DestroyServer(server);
                        }
                    }
                    finally
                    {
                        foreach (X509Certificate2 cert in certs)
                        {
                            certStore.Remove(cert);
                        }
                    }
                }
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing custom certificate verifier... ");
                Console.Out.Flush();
                {
                    // Verify that a server certificate is present.
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");

                    bool invoked = false;
                    bool hadCert = false;
                    await using var comm = new Communicator(ref args, clientProperties,
                        tlsClientOptions: new TlsClientOptions()
                        {
#pragma warning disable CA5359 // Do Not Disable Certificate Validation
                            ServerCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                            {
                                hadCert = certificate != null;
                                invoked = true;
                                return true;
                            }
#pragma warning restore CA5359 // Do Not Disable Certificate Validation
                        });

                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        TestHelper.Assert(server != null);
                        var tcpConnection = (TcpConnection)await server.GetConnectionAsync();
                        TestHelper.Assert(tcpConnection.IsEncrypted);
                        server.CheckCipher(tcpConnection.NegotiatedCipherSuite!.ToString()!);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    TestHelper.Assert(invoked);
                    TestHelper.Assert(hadCert);
                    fact.DestroyServer(server);
                }

                {
                    bool hadCert = false;
                    bool invoked = false;
                    // Have the verifier return false. Close the connection explicitly to force a new connection to be
                    // established.
                    await using var comm = new Communicator(ref args, clientProperties,
                        tlsClientOptions: new TlsClientOptions()
                        {
                            ServerCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) =>
                            {
                                hadCert = certificate != null;
                                invoked = true;
                                return false;
                            }
                        });
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        TestHelper.Assert(server != null);
                        server.IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (TransportException)
                    {
                        // Expected.
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    TestHelper.Assert(invoked);
                    TestHelper.Assert(hadCert);
                    fact.DestroyServer(server);
                }
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing protocols... ");
                Console.Out.Flush();
                {
                    // This should fail because the client and server have no protocol
                    // in common.
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                    clientProperties["IceSSL.Protocols"] = "tls1_2";
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    serverProperties["IceSSL.Protocols"] = "tls1_3";
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        server.IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (TransportException)
                    {
                        // Expected.
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }
                {
                    // This should succeed.
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    serverProperties["IceSSL.Protocols"] = "tls1_2, tls1_3";
                    IServerPrx server = fact.CreateServer(serverProperties, true);

                    server.IcePing();
                    fact.DestroyServer(server);
                }

                try
                {
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                    clientProperties["IceSSL.Protocols"] = "tls1_2";
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    serverProperties["IceSSL.Protocols"] = "tls1_2";
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    server.IcePing();

                    fact.DestroyServer(server);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.ToString());
                    TestHelper.Assert(false);
                }
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing expired certificates... ");
                Console.Out.Flush();
                {
                    // This should fail because the server's certificate is expired.
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1_exp", "cacert1");
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        server.IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (TransportException)
                    {
                        // Expected.
                    }
                    catch (Exception ex)
                    {
                        Console.Out.Write(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }
                {
                    // This should fail because the client's certificate is expired.
                    clientProperties["IceSSL.CertFile"] = "c_rsa_ca1_exp.p12";
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        server.IcePing();
                        TestHelper.Assert(false);
                    }
                    catch (ConnectionLostException)
                    {
                        // Expected.
                    }
                    catch (Exception ex)
                    {
                        Console.Out.Write(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }
                Console.Out.WriteLine("ok");

                if (OperatingSystem.IsWindows() && isAdministrator)
                {
                    // LocalMachine certificate store is not supported on non Windows platforms.
                    Console.Out.Write("testing multiple CA certificates... ");
                    Console.Out.Flush();
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");
                    await using var comm = new Communicator(ref args, clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca2");
                    store.Add(caCert1);
                    store.Add(caCert2);
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        server.IcePing();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                    store.Remove(caCert1);
                    store.Remove(caCert2);
                    Console.Out.WriteLine("ok");
                }

                Console.Out.Write("testing multiple CA certificates... ");
                Console.Out.Flush();
                {
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacerts");
                    await using var comm = new Communicator(clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca2", "cacerts");
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        server.IcePing();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing DER CA certificate... ");
                Console.Out.Flush();
                {
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");
                    clientProperties["IceSSL.CAs"] = "cacert1.der";
                    await using var comm = new Communicator(clientProperties);
                    var fact = CreateServerFactoryPrx(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                    serverProperties["IceSSL.CAs"] = "cacert1.der";
                    IServerPrx server = fact.CreateServer(serverProperties, true);
                    try
                    {
                        server.IcePing();
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine(ex.ToString());
                        TestHelper.Assert(false);
                    }
                    fact.DestroyServer(server);
                }
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing passwords... ");
                Console.Out.Flush();
                {
                    // Test password failure.
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");
                    // Don't specify the password.
                    clientProperties.Remove("IceSSL.Password");
                    try
                    {
                        _ = new Communicator(ref args, clientProperties);
                        TestHelper.Assert(false);
                    }
                    catch
                    {
                        // Expected.
                    }
                }
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing system CAs... ");
                Console.Out.Flush();
                {
                    // Retry a few times in case there are connectivity problems with demo.zeroc.com.
                    const int retryMax = 5;
                    const int retryDelay = 1000;
                    int retryCount = 0;
                    clientProperties = CreateProperties(defaultProperties);
                    clientProperties["IceSSL.DefaultDir"] = "";
                    clientProperties["Ice.Override.Timeout"] = "5000"; // 5s timeout
                    await using var comm = new Communicator(clientProperties);
                    var p = IObjectPrx.Parse("dummy:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2", comm);

                    while (true)
                    {
                        try
                        {
                            _ = await p.GetConnectionAsync();
                            break;
                        }
                        catch (Exception ex)
                        {
                            if ((ex is ConnectTimeoutException) ||
                                (ex is TransportException) ||
                                (ex is DNSException))
                            {
                                if (++retryCount < retryMax)
                                {
                                    Console.Out.Write("retrying... ");
                                    Console.Out.Flush();
                                    Thread.Sleep(retryDelay);
                                    continue;
                                }
                            }

                            Console.Out.WriteLine("warning: unable to connect to demo.zeroc.com to check system CA");
                            Console.WriteLine(ex.ToString());
                            break;
                        }
                    }
                }
                Console.Out.WriteLine("ok");
            }
            finally
            {
                if (OperatingSystem.IsWindows() && isAdministrator)
                {
                    store.Remove(caCert1);
                    store.Remove(caCert2);
                }
                store.Close();
            }

            await factory.ShutdownAsync();
        }
    }
}
