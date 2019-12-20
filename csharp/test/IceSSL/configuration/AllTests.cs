//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

//
// NOTE: This test is not interoperable with other language mappings.
//

using System;
using System.Diagnostics;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Threading;
using Ice;
public class AllTests
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new System.Exception();
        }
    }

    private static X509Certificate2 createCertificate(string certPEM)
    {
        return new X509Certificate2(System.Text.Encoding.ASCII.GetBytes(certPEM));
    }

    private static Dictionary<string, string>
    CreateProperties(Dictionary<string, string> defaultProperties, string? cert = null, string? ca = null)
    {
        var properties = new Dictionary<string, string>(defaultProperties);
        properties["Ice.Plugin.IceSSL"] = "IceSSL:IceSSL.PluginFactory";
        string? value;
        if (defaultProperties.TryGetValue("IceSSL.DefaultDir", out value))
        {
            properties["IceSSL.DefaultDir"] = value;
        }

        if (defaultProperties.TryGetValue("Ice.Default.Host", out value))
        {
            properties["Ice.Default.Host"] = value;
        }

        if (defaultProperties.TryGetValue("Ice.IPv6", out value))
        {
            properties["Ice.IPv6"] = value;
        }

        properties["Ice.RetryIntervals"] = "-1";
        //properties["IceSSL.Trace.Security"] = "1";

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

    public static Test.ServerFactoryPrx allTests(Test.TestHelper helper, string testDir)
    {
        Ice.Communicator communicator = helper.communicator();
        string factoryRef = "factory:" + helper.getTestEndpoint(0, "tcp");

        var factory = Test.ServerFactoryPrx.Parse(factoryRef, communicator);

        string defaultHost = communicator.GetProperty("Ice.Default.Host") ?? "";
        string defaultDir = $"{testDir}/../certs";
        Dictionary<string, string> defaultProperties = communicator.GetProperties();
        defaultProperties["IceSSL.DefaultDir"] = defaultDir;

        //
        // Load the CA certificates. We could use the IceSSL.ImportCert property, but
        // it would be nice to remove the CA certificates when the test finishes, so
        // this test manually installs the certificates in the LocalMachine:AuthRoot
        // store.
        //
        // Note that the client and server are assumed to run on the same machine,
        // so the certificates installed by the client are also available to the
        // server.
        //
        string caCert1File = defaultDir + "/cacert1.pem";
        string caCert2File = defaultDir + "/cacert2.pem";
        X509Certificate2 caCert1 = new X509Certificate2(caCert1File);
        X509Certificate2 caCert2 = new X509Certificate2(caCert2File);

        test(Enumerable.SequenceEqual(createCertificate(File.ReadAllText(caCert1File)).RawData, caCert1.RawData));
        test(Enumerable.SequenceEqual(createCertificate(File.ReadAllText(caCert2File)).RawData, caCert2.RawData));

        X509Store store = new X509Store(StoreName.AuthRoot, StoreLocation.LocalMachine);
        bool isAdministrator = false;
        if (IceInternal.AssemblyUtil.isWindows)
        {
            try
            {
                store.Open(OpenFlags.ReadWrite);
                isAdministrator = true;
            }
            catch (CryptographicException)
            {
                store.Open(OpenFlags.ReadOnly);
                Console.Out.WriteLine("warning: some test requires administrator privileges, run as Administrator to run all the tests.");
            }
        }

        Dictionary<string, string> clientProperties;
        Dictionary<string, string> serverProperties;
        try
        {
            string[] args = new string[0];

            Console.Out.Write("testing manual initialization... ");
            Console.Out.Flush();
            {
                clientProperties = CreateProperties(defaultProperties);
                clientProperties["Ice.InitPlugins"] = "0";
                var comm = new Communicator(ref args, clientProperties);
                var p = IObjectPrx.Parse("dummy:ssl -p 9999", comm);
                try
                {
                    p.IcePing();
                    test(false);
                }
                catch (System.Exception)
                {
                }
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["Ice.InitPlugins"] = "0";
                var comm = new Communicator(ref args, clientProperties);
                comm.InitializePlugins();
                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                //
                // Supply our own certificate.
                //
                X509Certificate2 cert = new X509Certificate2(defaultDir + "/c_rsa_ca1.p12", "password");
                X509Certificate2Collection coll = new X509Certificate2Collection();
                coll.Add(cert);
                clientProperties = CreateProperties(defaultProperties);
                clientProperties["Ice.InitPlugins"] = "0";
                clientProperties["IceSSL.CAs"] = caCert1File;
                var comm = new Communicator(ref args, clientProperties);
                IceSSL.Plugin? plugin = (IceSSL.Plugin?)comm.GetPlugin("IceSSL");
                Debug.Assert(plugin != null);
                plugin.setCertificates(coll);
                comm.InitializePlugins();
                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.VerifyPeer"] = "2";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }

            {
                //
                // Supply our own CA certificate.
                //
                X509Certificate2 cert = new X509Certificate2(defaultDir + "/cacert1.pem");
                X509Certificate2Collection coll = new X509Certificate2Collection();
                coll.Add(cert);
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");
                clientProperties["Ice.InitPlugins"] = "0";
                var comm = new Communicator(ref args, clientProperties);
                IceSSL.Plugin? plugin = (IceSSL.Plugin?)comm.GetPlugin("IceSSL");
                Debug.Assert(plugin != null);
                plugin.setCACertificates(coll);
                comm.InitializePlugins();
                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.VerifyPeer"] = "2";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing certificate verification... ");
            Console.Out.Flush();
            {
                //
                // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
                // and it doesn't trust the server certificate.
                //
                clientProperties = CreateProperties(defaultProperties);
                clientProperties["IceSSL.VerifyPeer"] = "0";
                var comm = new Communicator(ref args, clientProperties);
                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                serverProperties["IceSSL.VerifyPeer"] = "0";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.noCert();
                    test(!((IceSSL.ConnectionInfo)server.GetConnection().getInfo()).verified);
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();

                //
                // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
                // but it still verifies the server's.
                //
                clientProperties = CreateProperties(defaultProperties, ca: "cacert1");
                clientProperties["IceSSL.VerifyPeer"] = "0";
                comm = new Communicator(ref args, clientProperties);
                fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                serverProperties["IceSSL.VerifyPeer"] = "0";
                server = fact.createServer(serverProperties);
                try
                {
                    server.noCert();
                    test(((IceSSL.ConnectionInfo)server.GetConnection().getInfo()).verified);
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();

                //
                // Test IceSSL.VerifyPeer=1. Client does not have a certificate.
                //
                clientProperties = CreateProperties(defaultProperties, ca: "cacert1");
                comm = new Communicator(ref args, clientProperties);
                fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                serverProperties["IceSSL.VerifyPeer"] = "1";
                server = fact.createServer(serverProperties);
                try
                {
                    server.noCert();
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);

                //
                // Test IceSSL.VerifyPeer=2. This should fail because the client
                // does not supply a certificate.
                //
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                serverProperties["IceSSL.VerifyPeer"] = "2";
                server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (ConnectionLostException)
                {
                    // Expected.
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);

                comm.destroy();

                //
                // Test IceSSL.VerifyPeer=1. Client has a certificate.
                //
                // Provide "cacert1" to the client to verify the server
                // certificate (without this the client connection wouln't be
                // able to provide the certificate chain).
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                comm = new Communicator(ref args, clientProperties);
                fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.VerifyPeer"] = "1";
                server = fact.createServer(serverProperties);
                try
                {
                    X509Certificate2 clientCert =
                        new X509Certificate2(defaultDir + "/c_rsa_ca1.p12", "password");
                    server.checkCert(clientCert.Subject, clientCert.Issuer);

                    X509Certificate2 serverCert =
                        new X509Certificate2(defaultDir + "/s_rsa_ca1.p12", "password");
                    X509Certificate2 caCert = new X509Certificate2(defaultDir + "/cacert1.pem");

                    IceSSL.ConnectionInfo info = (IceSSL.ConnectionInfo)server.GetConnection().getInfo();
                    test(info.certs!.Length == 2);
                    test(info.verified);

                    test(caCert.Equals(info.certs[1]));
                    test(serverCert.Equals(info.certs[0]));
                }
                catch (System.Exception ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);

                //
                // Test IceSSL.VerifyPeer=2. Client has a certificate.
                //
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.VerifyPeer"] = "2";
                server = fact.createServer(serverProperties);
                try
                {
                    X509Certificate2 clientCert = new X509Certificate2(defaultDir + "/c_rsa_ca1.p12", "password");
                    server.checkCert(clientCert.Subject, clientCert.Issuer);
                }
                catch (System.Exception ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);

                comm.destroy();

                //
                // Test IceSSL.VerifyPeer=1. This should fail because the
                // client doesn't trust the server's CA.
                //
                clientProperties = CreateProperties(defaultProperties);
                comm = new Communicator(ref args, clientProperties);
                fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                serverProperties["IceSSL.VerifyPeer"] = "0";
                server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (SecurityException)
                {
                    // Expected.
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();

                //
                // Test IceSSL.VerifyPeer=1. This should fail because the
                // server doesn't trust the client's CA.
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca2");
                clientProperties["IceSSL.VerifyPeer"] = "0";
                comm = new Communicator(ref args, clientProperties);
                fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                serverProperties["IceSSL.VerifyPeer"] = "1";
                server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (SecurityException)
                {
                    // Expected.
                }
                catch (ConnectionLostException)
                {
                    // Expected.
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();

                //
                // This should succeed because the self signed certificate used by the server is
                // trusted.
                //
                clientProperties = CreateProperties(defaultProperties, ca: "cacert2");
                comm = new Communicator(ref args, clientProperties);
                fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "cacert2");
                serverProperties["IceSSL.VerifyPeer"] = "0";
                server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();

                //
                // This should l because the self signed certificate used by the server is not
                // trusted.
                //
                clientProperties = CreateProperties(defaultProperties);
                comm = new Communicator(ref args, clientProperties);
                fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "cacert2");
                serverProperties["IceSSL.VerifyPeer"] = "0";
                server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (SecurityException)
                {
                    // Expected.
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();

                //
                // Verify that IceSSL.CheckCertName has no effect in a server.
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                comm = new Communicator(ref args, clientProperties);
                fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.CheckCertName"] = "1";
                server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();

                //
                // Test Hostname verification only when Ice.DefaultHost is 127.0.0.1
                // as that is the IP address used in the test certificates.
                //
                if (defaultHost.Equals("127.0.0.1"))
                {
                    //
                    // Test using localhost as target host
                    //
                    var props = new Dictionary<string, string>(defaultProperties);
                    props["Ice.Default.Host"] = "localhost";

                    //
                    // Target host matches the certificate DNS altName
                    //
                    {
                        clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                        clientProperties["IceSSL.CheckCertName"] = "1";
                        comm = new Communicator(ref args, clientProperties);

                        fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                        serverProperties = CreateProperties(props, "s_rsa_ca1_cn1", "cacert1");
                        serverProperties["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(serverProperties);
                        try
                        {
                            server.IcePing();
                        }
                        catch (Ice.LocalException ex)
                        {
                            //
                            // macOS catalina does not check the certificate common name
                            //
                            if (!IceInternal.AssemblyUtil.isMacOS)
                            {
                                Console.WriteLine(ex.ToString());
                                test(false);
                            }
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }
                    //
                    // Target host does not match the certificate DNS altName
                    //
                    {
                        clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                        clientProperties["IceSSL.CheckCertName"] = "1";
                        comm = new Communicator(ref args, clientProperties);

                        fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                        serverProperties = CreateProperties(props, "s_rsa_ca1_cn2", "cacert1");
                        serverProperties["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(serverProperties);
                        try
                        {
                            server.IcePing();
                            test(false);
                        }
                        catch (SecurityException)
                        {
                            // Expected
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }
                    //
                    // Target host matches the certificate Common Name and the certificate does not
                    // include a DNS altName
                    //
                    {
                        clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                        clientProperties["IceSSL.CheckCertName"] = "1";
                        comm = new Communicator(ref args, clientProperties);

                        fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                        serverProperties = CreateProperties(props, "s_rsa_ca1_cn3", "cacert1");
                        serverProperties["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(serverProperties);
                        try
                        {
                            server.IcePing();
                        }
                        catch (Ice.LocalException ex)
                        {
                            Console.WriteLine(ex.ToString());
                            test(false);
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }
                    //
                    // Target host does not match the certificate Common Name and the certificate does not
                    // include a DNS altName
                    //
                    {
                        clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                        clientProperties["IceSSL.CheckCertName"] = "1";
                        comm = new Communicator(ref args, clientProperties);

                        fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                        serverProperties = CreateProperties(props, "s_rsa_ca1_cn4", "cacert1");
                        serverProperties["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(serverProperties);
                        try
                        {
                            server.IcePing();
                            test(false);
                        }
                        catch (SecurityException)
                        {
                            // Expected
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }
                    //
                    // Target host matches the certificate Common Name and the certificate has
                    // a DNS altName that does not matches the target host
                    //
                    {
                        clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                        clientProperties["IceSSL.CheckCertName"] = "1";
                        comm = new Communicator(ref args, clientProperties);

                        fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                        serverProperties = CreateProperties(props, "s_rsa_ca1_cn5", "cacert1");
                        serverProperties["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(serverProperties);
                        try
                        {
                            server.IcePing();
                            test(false);
                        }
                        catch (Ice.SecurityException)
                        {
                            // Expected
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }

                    //
                    // Test using 127.0.0.1 as target host
                    //

                    //
                    // Disabled for compatibility with older Windows
                    // versions.
                    //
                    /* //
                    // Target host matches the certificate IP altName
                    //
                    {
                        clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                        clientProperties["IceSSL.CheckCertName", "1");
                        comm = new Communicator(ref args, initData);

                        fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                        test(fact != null);
                        serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1_cn6", "cacert1");
                        d["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(d);
                        try
                        {
                            server.IcePing();
                        }
                        catch(Ice.LocalException)
                        {
                            test(false);
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }
                    //
                    // Target host does not match the certificate IP altName
                    //
                    {
                        clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                        clientProperties["IceSSL.CheckCertName", "1");
                        comm = new Communicator(ref args, initData);

                        fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                        test(fact != null);
                        serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1_cn7", "cacert1");
                        d["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(d);
                        try
                        {
                            server.IcePing();
                            test(false);
                        }
                        catch(Ice.SecurityException)
                        {
                            // Expected
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }*/
                    //
                    // Target host is an IP addres that matches the CN and the certificate doesn't
                    // include an IP altName.
                    //
                    {
                        clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                        clientProperties["IceSSL.CheckCertName"] = "1";
                        comm = new Communicator(ref args, clientProperties);

                        fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                        serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1_cn8", "cacert1");
                        serverProperties["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(serverProperties);
                        try
                        {
                            server.IcePing();
                        }
                        catch (SecurityException ex)
                        {
                            //
                            // macOS catalina does not check the certificate common name
                            //
                            if (!IceInternal.AssemblyUtil.isMacOS)
                            {
                                Console.WriteLine(ex.ToString());
                                test(false);
                            }
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }

                    //
                    // Target host does not match the certificate DNS altName, connection should succeed
                    // because IceSSL.VerifyPeer is set to 0.
                    //
                    {
                        clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                        clientProperties["IceSSL.CheckCertName"] = "1";
                        clientProperties["IceSSL.VerifyPeer"] = "0";
                        comm = new Communicator(ref args, clientProperties);

                        fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                        serverProperties = CreateProperties(props, "s_rsa_ca1_cn2", "cacert1");
                        server = fact.createServer(serverProperties);
                        try
                        {
                            server.IcePing();
                            IceSSL.ConnectionInfo info = (IceSSL.ConnectionInfo)server.GetConnection().getInfo();
                            test(!info.verified);
                        }
                        catch (LocalException ex)
                        {
                            Console.WriteLine(ex.ToString());
                            test(false);
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }

                    //
                    // Target host does not match the certificate DNS altName, connection should succeed
                    // because IceSSL.CheckCertName is set to 0.
                    //
                    {
                        clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                        clientProperties["IceSSL.CheckCertName"] = "0";
                        comm = new Communicator(ref args, clientProperties);

                        fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                        serverProperties = CreateProperties(props, "s_rsa_ca1_cn2", "cacert1");
                        serverProperties["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(serverProperties);
                        try
                        {
                            server.IcePing();
                        }
                        catch (LocalException ex)
                        {
                            Console.WriteLine(ex.ToString());
                            test(false);
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }
                }
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing certificate chains... ");
            Console.Out.Flush();
            {
                X509Store certStore = new X509Store("My", StoreLocation.CurrentUser);
                certStore.Open(OpenFlags.ReadWrite);
                X509Certificate2Collection certs = new X509Certificate2Collection();
                var storageFlags = X509KeyStorageFlags.DefaultKeySet;
                if (IceInternal.AssemblyUtil.isMacOS)
                {
                    //
                    // On macOS, we need to mark the key exportable because the addition of the key to the
                    // cert store requires to move the key from on keychain to another (which requires the
                    // Exportable flag... see https://github.com/dotnet/corefx/issues/25631)
                    //
                    storageFlags |= X509KeyStorageFlags.Exportable;
                }
                certs.Import(defaultDir + "/s_rsa_cai2.p12", "password", storageFlags);
                foreach (X509Certificate2 cert in certs)
                {
                    certStore.Add(cert);
                }
                try
                {
                    IceSSL.ConnectionInfo info;

                    clientProperties = CreateProperties(defaultProperties);
                    clientProperties["IceSSL.VerifyPeer"] = "0";
                    Communicator comm = new Communicator(clientProperties);

                    Test.ServerFactoryPrx fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);

                    //
                    // The client can't verify the server certificate but it should
                    // still provide it. "s_rsa_ca1" doesn't include the root so the
                    // cert size should be 1.
                    //
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                    serverProperties["IceSSL.VerifyPeer"] = "0";
                    Test.ServerPrx server = fact.createServer(serverProperties);
                    try
                    {
                        info = (IceSSL.ConnectionInfo)server.GetConnection().getInfo();
                        test(info.certs!.Length == 1);
                        test(!info.verified);
                    }
                    catch (Ice.LocalException ex)
                    {
                        Console.WriteLine(ex.ToString());
                        test(false);
                    }
                    fact.destroyServer(server);

                    //
                    // Setting the CA for the server shouldn't change anything, it
                    // shouldn't modify the cert chain sent to the client.
                    //
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    serverProperties["IceSSL.VerifyPeer"] = "0";
                    server = fact.createServer(serverProperties);
                    try
                    {
                        info = (IceSSL.ConnectionInfo)server.GetConnection().getInfo();
                        test(info.certs!.Length == 1);
                        test(!info.verified);
                    }
                    catch (Ice.LocalException ex)
                    {
                        Console.WriteLine(ex.ToString());
                        test(false);
                    }
                    fact.destroyServer(server);

                    //
                    // The client can't verify the server certificate but should
                    // still provide it. "s_rsa_wroot_ca1" includes the root so
                    // the cert size should be 2.
                    //
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_wroot_ca1");
                    serverProperties["IceSSL.VerifyPeer"] = "0";
                    server = fact.createServer(serverProperties);
                    try
                    {
                        info = (IceSSL.ConnectionInfo)server.GetConnection().getInfo();
                        test(info.certs!.Length == 1); // Like the SChannel transport, .NET never sends the root.
                    }
                    catch (Ice.LocalException ex)
                    {
                        Console.WriteLine(ex.ToString());
                        test(false);
                    }
                    fact.destroyServer(server);
                    comm.destroy();

                    //
                    // Now the client verifies the server certificate
                    //
                    clientProperties = CreateProperties(defaultProperties, ca: "cacert1");
                    clientProperties["IceSSL.VerifyPeer"] = "1";
                    comm = new Communicator(clientProperties);

                    fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);

                    {
                        serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                        serverProperties["IceSSL.VerifyPeer"] = "0";
                        server = fact.createServer(serverProperties);
                        try
                        {
                            info = (IceSSL.ConnectionInfo)server.GetConnection().getInfo();
                            test(info.certs!.Length == 2);
                            test(info.verified);
                        }
                        catch (LocalException ex)
                        {
                            Console.WriteLine(ex.ToString());
                            test(false);
                        }
                        fact.destroyServer(server);
                    }

                    //
                    // Try certificate with one intermediate and VerifyDepthMax=2
                    //
                    clientProperties = CreateProperties(defaultProperties, ca: "cacert1");
                    clientProperties["IceSSL.VerifyPeer"] = "1";
                    clientProperties["IceSSL.VerifyDepthMax"] = "2";
                    comm = new Communicator(clientProperties);

                    fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);

                    {
                        serverProperties = CreateProperties(defaultProperties, "s_rsa_cai1");
                        serverProperties["IceSSL.VerifyPeer"] = "0";
                        server = fact.createServer(serverProperties);
                        try
                        {
                            server.GetConnection().getInfo();
                            test(false);
                        }
                        catch (Ice.SecurityException)
                        {
                            // Chain length too long
                        }
                        catch (Ice.LocalException ex)
                        {
                            Console.WriteLine(ex.ToString());
                            test(false);
                        }
                        fact.destroyServer(server);
                    }
                    comm.destroy();

                    if (IceInternal.AssemblyUtil.isWindows)
                    {
                        //
                        // The certificate chain on Linux doesn't include the intermeidate
                        // certificates see ICE-8576
                        //

                        //
                        // Set VerifyDepthMax to 3 (the default)
                        //
                        clientProperties = CreateProperties(defaultProperties, ca: "cacert1");
                        clientProperties["IceSSL.VerifyPeer"] = "1";
                        //clientProperties["IceSSL.VerifyDepthMax", "3");
                        comm = new Communicator(clientProperties);

                        fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);

                        {
                            serverProperties = CreateProperties(defaultProperties, "s_rsa_cai1");
                            serverProperties["IceSSL.VerifyPeer"] = "0";
                            server = fact.createServer(serverProperties);
                            try
                            {
                                info = (IceSSL.ConnectionInfo)server.GetConnection().getInfo();
                                test(info.certs!.Length == 3);
                                test(info.verified);
                            }
                            catch (Ice.LocalException ex)
                            {
                                Console.WriteLine(ex.ToString());
                                test(false);
                            }
                            fact.destroyServer(server);
                        }

                        {
                            serverProperties = CreateProperties(defaultProperties, "s_rsa_cai2");
                            serverProperties["IceSSL.VerifyPeer"] = "0";
                            server = fact.createServer(serverProperties);
                            try
                            {
                                server.GetConnection().getInfo();
                                test(false);
                            }
                            catch (Ice.SecurityException)
                            {
                                // Chain length too long
                            }
                            fact.destroyServer(server);
                        }
                        comm.destroy();

                        //
                        // Increase VerifyDepthMax to 4
                        //
                        clientProperties = CreateProperties(defaultProperties, ca: "cacert1");
                        clientProperties["IceSSL.VerifyPeer"] = "1";
                        clientProperties["IceSSL.VerifyDepthMax"] = "4";
                        comm = new Communicator(clientProperties);

                        fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);

                        {
                            serverProperties = CreateProperties(defaultProperties, "s_rsa_cai2");
                            serverProperties["IceSSL.VerifyPeer"] = "0";
                            server = fact.createServer(serverProperties);
                            try
                            {
                                info = (IceSSL.ConnectionInfo)server.GetConnection().getInfo();
                                test(info.certs!.Length == 4);
                                test(info.verified);
                            }
                            catch (Ice.LocalException ex)
                            {
                                Console.WriteLine(ex.ToString());
                                test(false);
                            }
                            fact.destroyServer(server);
                        }

                        comm.destroy();

                        //
                        // Increase VerifyDepthMax to 4
                        //
                        clientProperties = CreateProperties(defaultProperties, "c_rsa_cai2", "cacert1");
                        clientProperties["IceSSL.VerifyPeer"] = "1";
                        clientProperties["IceSSL.VerifyDepthMax"] = "4";
                        comm = new Communicator(clientProperties);

                        fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                        {
                            serverProperties = CreateProperties(defaultProperties, "s_rsa_cai2", "cacert1");
                            serverProperties["IceSSL.VerifyPeer"] = "2";
                            server = fact.createServer(serverProperties);
                            try
                            {
                                server.GetConnection();
                                test(false);
                            }
                            catch (ProtocolException)
                            {
                                // Expected
                            }
                            catch (ConnectionLostException)
                            {
                                // Expected
                            }
                            catch (LocalException ex)
                            {
                                Console.WriteLine(ex.ToString());
                                test(false);
                            }
                            fact.destroyServer(server);
                        }

                        {
                            serverProperties = CreateProperties(defaultProperties, "s_rsa_cai2", "cacert1");
                            serverProperties["IceSSL.VerifyPeer"] = "2";
                            serverProperties["IceSSL.VerifyDepthMax"] = "4";
                            server = fact.createServer(serverProperties);
                            try
                            {
                                server.GetConnection();
                            }
                            catch (LocalException ex)
                            {
                                Console.WriteLine(ex.ToString());
                                test(false);
                            }
                            fact.destroyServer(server);
                        }

                        comm.destroy();
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
                //
                // Verify that a server certificate is present.
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);
                IceSSL.Plugin? plugin = (IceSSL.Plugin?)comm.GetPlugin("IceSSL");
                Debug.Assert(plugin != null);
                CertificateVerifierI verifier = new CertificateVerifierI();
                plugin.setCertificateVerifier(verifier);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.VerifyPeer"] = "2";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    IceSSL.ConnectionInfo info = (IceSSL.ConnectionInfo)server.GetConnection().getInfo();
                    Debug.Assert(info.cipher != null);
                    server.checkCipher(info.cipher);
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                test(verifier.invoked());
                test(verifier.hadCert());

                //
                // Have the verifier return false. Close the connection explicitly
                // to force a new connection to be established.
                //
                verifier.reset();
                verifier.returnValue(false);
                server.GetConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (SecurityException)
                {
                    // Expected.
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                test(verifier.invoked());
                test(verifier.hadCert());
                fact.destroyServer(server);

                comm.destroy();
            }
            {
                //
                // Verify that verifier is installed via property.
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");
                clientProperties["IceSSL.CertVerifier"] = "CertificateVerifierI";
                Communicator comm = new Communicator(ref args, clientProperties);
                IceSSL.Plugin? plugin = (IceSSL.Plugin?)comm.GetPlugin("IceSSL");
                Debug.Assert(plugin != null);
                test(plugin.getCertificateVerifier() != null);
                comm.destroy();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing protocols... ");
            Console.Out.Flush();
            {
                //
                // This should fail because the client and server have no protocol
                // in common.
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.Protocols"] = "tls1_1";
                Communicator comm = new Communicator(ref args, clientProperties);
                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.VerifyPeer"] = "2";
                serverProperties["IceSSL.Protocols"] = "tls1_2";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    //test(false);
                }
                catch (SecurityException)
                {
                    // Expected.
                }
                catch (ConnectionLostException)
                {
                    // Expected.
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();

                //
                // This should succeed.
                //
                comm = new Communicator(ref args, clientProperties);
                fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.VerifyPeer"] = "2";
                serverProperties["IceSSL.Protocols"] = "tls1_1, tls1_2";
                server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    if (ex.ToString().IndexOf("no protocols available") < 0) // Expected if TLS1.1 is disabled (RHEL8)
                    {
                        Console.WriteLine(ex.ToString());
                        test(false);
                    }
                }
                fact.destroyServer(server);
                comm.destroy();

                try
                {
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                    clientProperties["IceSSL.Protocols"] = "tls1_2";
                    comm = new Communicator(ref args, clientProperties);
                    fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                    serverProperties["IceSSL.VerifyPeer"] = "2";
                    serverProperties["IceSSL.Protocols"] = "tls1_2";
                    server = fact.createServer(serverProperties);
                    server.IcePing();

                    fact.destroyServer(server);
                    comm.destroy();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing expired certificates... ");
            Console.Out.Flush();
            {
                //
                // This should fail because the server's certificate is expired.
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);
                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1_exp", "cacert1");
                serverProperties["IceSSL.VerifyPeer"] = "2";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (SecurityException)
                {
                    // Expected.
                }
                catch (LocalException ex)
                {
                    Console.Out.Write(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();

                //
                // This should fail because the client's certificate is expired.
                //
                clientProperties["IceSSL.CertFile"] = "c_rsa_ca1_exp.p12";
                comm = new Communicator(ref args, clientProperties);
                fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.VerifyPeer"] = "2";
                server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (ConnectionLostException)
                {
                    // Expected.
                }
                catch (LocalException ex)
                {
                    Console.Out.Write(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            Console.Out.WriteLine("ok");

            if (IceInternal.AssemblyUtil.isWindows && isAdministrator)
            {
                //
                // LocalMachine certificate store is not supported on non
                // Windows platforms.
                //
                Console.Out.Write("testing multiple CA certificates... ");
                Console.Out.Flush();
                {
                    clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");
                    clientProperties["IceSSL.UsePlatformCAs"] = "1";
                    Communicator comm = new Communicator(ref args, clientProperties);
                    var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                    serverProperties = CreateProperties(defaultProperties, "s_rsa_ca2");
                    serverProperties["IceSSL.VerifyPeer"] = "2";
                    serverProperties["IceSSL.UsePlatformCAs"] = "1";
                    store.Add(caCert1);
                    store.Add(caCert2);
                    Test.ServerPrx server = fact.createServer(serverProperties);
                    try
                    {
                        server.IcePing();
                    }
                    catch (LocalException ex)
                    {
                        Console.WriteLine(ex.ToString());
                        test(false);
                    }
                    fact.destroyServer(server);
                    store.Remove(caCert1);
                    store.Remove(caCert2);
                    comm.destroy();
                }
                Console.Out.WriteLine("ok");
            }

            Console.Out.Write("testing multiple CA certificates... ");
            Console.Out.Flush();
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacerts");
                Communicator comm = new Communicator(clientProperties);
                Test.ServerFactoryPrx fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca2", "cacerts");
                serverProperties["IceSSL.VerifyPeer"] = "2";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing DER CA certificate... ");
            Console.Out.Flush();
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");
                clientProperties["IceSSL.CAs"] = "cacert1.der";
                using var comm = new Communicator(clientProperties);
                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1");
                serverProperties["IceSSL.VerifyPeer"] = "2";
                serverProperties["IceSSL.CAs"] = "cacert1.der";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing passwords... ");
            Console.Out.Flush();
            {
                //
                // Test password failure.
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");
                // Don't specify the password.
                clientProperties.Remove("IceSSL.Password");
                try
                {
                    new Communicator(ref args, clientProperties);
                    test(false);
                }
                catch (System.Exception)
                {
                    // Expected.
                }
            }
            {
                //
                // Test password failure with callback.
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");
                clientProperties["Ice.InitPlugins"] = "0";
                // Don't specify the password.
                clientProperties.Remove("IceSSL.Password");
                using var comm = new Communicator(ref args, clientProperties);
                IceSSL.Plugin? plugin = (IceSSL.Plugin?)comm.GetPlugin("IceSSL");
                Debug.Assert(plugin != null);
                PasswordCallbackI cb = new PasswordCallbackI("bogus");
                plugin.setPasswordCallback(cb);
                try
                {
                    comm.InitializePlugins();
                    test(false);
                }
                catch (System.Exception)
                {
                    // Expected.
                }
            }
            {
                //
                // Test installation of password callback.
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");
                clientProperties["Ice.InitPlugins"] = "0";
                // Don't specify the password.
                clientProperties.Remove("IceSSL.Password");
                Communicator comm = new Communicator(ref args, clientProperties);
                IceSSL.Plugin? plugin = (IceSSL.Plugin?)comm.GetPlugin("IceSSL");
                Debug.Assert(plugin != null);
                PasswordCallbackI cb = new PasswordCallbackI();
                plugin.setPasswordCallback(cb);
                test(plugin.getPasswordCallback() == cb);
                try
                {
                    comm.InitializePlugins();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                comm.destroy();
            }
            {
                //
                // Test password callback property.
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1");
                clientProperties["IceSSL.PasswordCallback"] = "PasswordCallbackI";
                // Don't specify the password.
                clientProperties.Remove("IceSSL.Password");
                Communicator comm = new Communicator(ref args, clientProperties);
                IceSSL.Plugin? plugin = (IceSSL.Plugin?)comm.GetPlugin("IceSSL");
                Debug.Assert(plugin != null);
                test(plugin.getPasswordCallback() != null);
                comm.destroy();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing IceSSL.TrustOnly... ");
            Console.Out.Flush();
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly"] =
                    "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Server";
                Communicator comm = new Communicator(ref args, clientProperties);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly"] =
                    "!C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Server";
                Communicator comm = new Communicator(ref args, clientProperties);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly"] =
                    "C=US, ST=Florida, O=\"ZeroC, Inc.\",OU=Ice, emailAddress=info@zeroc.com, CN=Server";
                Communicator comm = new Communicator(ref args, clientProperties);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly"] =
                    "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly"] =
                    "!C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly"] = "CN=Server";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly"] = "!CN=Server";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly"] = "CN=Client";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly"] = "!CN=Client";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly"] = "CN=Client";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly"] = "CN=Server";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly"] = "C=Canada,CN=Server";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly"] = "!C=Canada,CN=Server";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly"] = "C=Canada;CN=Server";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly"] = "!C=Canada;!CN=Server";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly"] = "!CN=Server1"; // Should not match "Server"
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly"] = "!CN=Client1"; // Should not match "Client"
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                //
                // Rejection takes precedence (client).
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly"] = "ST=Florida;!CN=Server;C=US";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                //
                // Rejection takes precedence (server).
                //
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly"] = "C=US;!CN=Client;ST=Florida";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing IceSSL.TrustOnly.Client... ");
            Console.Out.Flush();
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly.Client"] =
                    "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Server";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                // Should have no effect.
                serverProperties["IceSSL.TrustOnly.Client"] =
                    "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Server";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly.Client"] =
                    "!C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Server";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                // Should have no effect.
                serverProperties["IceSSL.TrustOnly.Client"] = "!CN=Client";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly.Client"] = "CN=Client";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                clientProperties["IceSSL.TrustOnly.Client"] = "!CN=Client";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing IceSSL.TrustOnly.Server... ");
            Console.Out.Flush();
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                // Should have no effect.
                clientProperties["IceSSL.TrustOnly.Server"] =
                    "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly.Server"] =
                    "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly.Server"] =
                    "!C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                // Should have no effect.
                clientProperties["IceSSL.TrustOnly.Server"] = "!CN=Server";
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly.Server"] = "CN=Server";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly.Server"] = "!CN=Client";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing IceSSL.TrustOnly.Server.<AdapterName>... ");
            Console.Out.Flush();
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly.Server"] = "CN=bogus";
                serverProperties["IceSSL.TrustOnly.Server.ServerAdapter"] =
                    "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly.Server.ServerAdapter"] =
                    "!C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly.Server.ServerAdapter"] = "CN=bogus";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                clientProperties = CreateProperties(defaultProperties, "c_rsa_ca1", "cacert1");
                Communicator comm = new Communicator(ref args, clientProperties);

                var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                serverProperties = CreateProperties(defaultProperties, "s_rsa_ca1", "cacert1");
                serverProperties["IceSSL.TrustOnly.Server.ServerAdapter"] = "!CN=bogus";
                Test.ServerPrx server = fact.createServer(serverProperties);
                try
                {
                    server.IcePing();
                }
                catch (Ice.LocalException ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing IceSSL.FindCerts properties... ");
            Console.Out.Flush();
            {
                string[] clientFindCertProperties = new string[]
                {
                    "SUBJECTDN:'CN=Client, OU=Ice, O=\"ZeroC, Inc.\", L=Jupiter, S=Florida, C=US, E=info@zeroc.com'",
                    "ISSUER:'ZeroC, Inc.' SUBJECT:Client SERIAL:02",
                    "ISSUERDN:'CN=ZeroC Test CA 1, OU=Ice, O=\"ZeroC, Inc.\",L=Jupiter, S=Florida, C=US,E=info@zeroc.com' SUBJECT:Client",
                    "THUMBPRINT:'82 30 1E 35 9E 39 C1 D0 63 0D 67 3D 12 DD D4 96 90 1E EF 54'",
                    "SUBJECTKEYID:'FC 5D 4F AB F0 6C 03 11 B8 F3 68 CF 89 54 92 3F F9 79 2A 06'"
                };

                string[] serverFindCertProperties = new string[]
                {
                    "SUBJECTDN:'CN=Server, OU=Ice, O=\"ZeroC, Inc.\", L=Jupiter, S=Florida, C=US, E=info@zeroc.com'",
                    "ISSUER:'ZeroC, Inc.' SUBJECT:Server SERIAL:01",
                    "ISSUERDN:'CN=ZeroC Test CA 1, OU=Ice, O=\"ZeroC, Inc.\", L=Jupiter, S=Florida, C=US,E=info@zeroc.com' SUBJECT:Server",
                    "THUMBPRINT:'C0 01 FF 9C C9 DA C8 0D 34 F6 2F DE 09 FB 28 0D 69 AB 78 BA'",
                    "SUBJECTKEYID:'47 84 AE F9 F2 85 3D 99 30 6A 03 38 41 1A B9 EB C3 9C B5 4D'"
                };

                string[] failFindCertProperties = new string[]
                {
                    "nolabel",
                    "unknownlabel:foo",
                    "LABEL:",
                    "SUBJECTDN:'CN = Client, E = infox@zeroc.com, OU = Ice, O = \"ZeroC, Inc.\", S = Florida, C = US'",
                    "ISSUER:'ZeroC, Inc.' SUBJECT:Client SERIAL:'02 02'",
                    "ISSUERDN:'E=info@zeroc.com, CN=ZeroC Test CA 1, OU=Ice, O=\"ZeroC, Inc.\"," +
                        " L=Jupiter, S=Florida, C=ES' SUBJECT:Client",
                    "THUMBPRINT:'27 e0 18 c9 23 12 6c f0 5c da fa 36 5a 4c 63 5a e2 53 07 ff'",
                    "SUBJECTKEYID:'a6 42 aa 17 04 41 86 56 67 e4 04 64 59 34 30 c7 4c 6b ef ff'"
                };

                string[] certificates = new string[] { "/s_rsa_ca1.p12", "/c_rsa_ca1.p12" };

                X509Store certStore = new X509Store("My", StoreLocation.CurrentUser);
                certStore.Open(OpenFlags.ReadWrite);
                var storageFlags = X509KeyStorageFlags.DefaultKeySet;
                if (IceInternal.AssemblyUtil.isMacOS)
                {
                    //
                    // On macOS, we need to mark the key exportable because the addition of the key to the
                    // cert store requires to move the key from on keychain to another (which requires the
                    // Exportable flag... see https://github.com/dotnet/corefx/issues/25631)
                    //
                    storageFlags |= X509KeyStorageFlags.Exportable;
                }
                try
                {
                    foreach (string cert in certificates)
                    {
                        certStore.Add(new X509Certificate2(defaultDir + cert, "password", storageFlags));
                    }

                    for (int i = 0; i < clientFindCertProperties.Length; ++i)
                    {
                        clientProperties = CreateProperties(defaultProperties, ca: "cacert1");
                        clientProperties["IceSSL.CertStore"] = "My";
                        clientProperties["IceSSL.CertStoreLocation"] = "CurrentUser";
                        clientProperties["IceSSL.FindCert"] = clientFindCertProperties[i];
                        //
                        // Use TrustOnly to ensure the peer has pick the expected certificate.
                        //
                        clientProperties["IceSSL.TrustOnly"] = "CN=Server";
                        Communicator comm = new Communicator(ref args, clientProperties);

                        var fact = Test.ServerFactoryPrx.Parse(factoryRef, comm);
                        serverProperties = CreateProperties(defaultProperties, ca: "cacert1");
                        // Use deprecated property here to test it
                        serverProperties["IceSSL.FindCert.CurrentUser.My"] = serverFindCertProperties[i];
                        //
                        // Use TrustOnly to ensure the peer has pick the expected certificate.
                        //
                        serverProperties["IceSSL.TrustOnly"] = "CN=Client";

                        Test.ServerPrx server = fact.createServer(serverProperties);
                        try
                        {
                            server.IcePing();
                        }
                        catch (Ice.LocalException ex)
                        {
                            Console.WriteLine(ex.ToString());
                            test(false);
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }

                    //
                    // These must fail because the search criteria does not match any certificates.
                    //
                    foreach (string s in failFindCertProperties)
                    {
                        try
                        {
                            clientProperties = CreateProperties(defaultProperties);
                            clientProperties["IceSSL.FindCert"] = s;
                            Communicator comm = new Communicator(ref args, clientProperties);
                            test(false);
                        }
                        catch (System.Exception)
                        {
                            // Expected
                        }
                    }

                }
                finally
                {
                    foreach (string cert in certificates)
                    {
                        certStore.Remove(new X509Certificate2(defaultDir + cert, "password"));
                    }
                    certStore.Close();
                }

                //
                // These must fail because we have already remove the certificates.
                //
                foreach (string s in clientFindCertProperties)
                {
                    try
                    {
                        clientProperties = CreateProperties(defaultProperties);
                        clientProperties["IceSSL.FindCert.CurrentUser.My"] = s;
                        Communicator comm = new Communicator(ref args, clientProperties);
                        test(false);
                    }
                    catch (System.Exception)
                    {
                        // Expected
                    }
                }
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing system CAs... ");
            Console.Out.Flush();
            {
                //
                // Retry a few times in case there are connectivity problems with demo.zeroc.com.
                //
                const int retryMax = 5;
                const int retryDelay = 1000;
                int retryCount = 0;

                clientProperties = CreateProperties(defaultProperties);
                clientProperties["IceSSL.DefaultDir"] = "";
                clientProperties["IceSSL.VerifyDepthMax"] = "4";
                clientProperties["Ice.Override.Timeout"] = "5000"; // 5s timeout
                if (IceInternal.AssemblyUtil.isWindows)
                {
                    //
                    // BUGFIX: SChannel TLS 1.2 bug that affects Windows versions prior to Windows 10
                    // can cause SSL handshake errors when connecting to the remote zeroc server.
                    //
                    clientProperties["IceSSL.Protocols"] = "TLS1_0,TLS1_1";
                }
                Communicator comm = new Communicator(clientProperties);
                var p = IObjectPrx.Parse("dummy:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2", comm);
                while (true)
                {
                    try
                    {
                        p.IcePing();
                        test(false);
                    }
                    catch (Ice.SecurityException)
                    {
                        // Expected, by default we don't check for system CAs.
                        break;
                    }
                    catch (Ice.LocalException ex)
                    {
                        if ((ex is Ice.ConnectTimeoutException) ||
                           (ex is Ice.SocketException) ||
                           (ex is Ice.DNSException))
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
                comm.destroy();

                retryCount = 0;
                clientProperties = CreateProperties(defaultProperties);
                clientProperties["IceSSL.DefaultDir"] = "";
                clientProperties["IceSSL.VerifyDepthMax"] = "4";
                clientProperties["Ice.Override.Timeout"] = "5000"; // 5s timeout
                clientProperties["IceSSL.UsePlatformCAs"] = "1";
                if (IceInternal.AssemblyUtil.isWindows)
                {
                    //
                    // BUGFIX: SChannel TLS 1.2 bug that affects Windows versions prior to Windows 10
                    // can cause SSL handshake errors when connecting to the remote zeroc server.
                    //
                    clientProperties["IceSSL.Protocols"] = "TLS1_0,TLS1_1";
                }
                comm = new Communicator(clientProperties);
                p = IObjectPrx.Parse("dummy:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2", comm);
                while (true)
                {
                    try
                    {
                        IceSSL.ConnectionInfo? info = (IceSSL.ConnectionInfo?)p.GetConnection().getInfo().underlying;
                        Debug.Assert(info != null);
                        test(info.verified);
                        break;
                    }
                    catch (Ice.LocalException ex)
                    {
                        if ((ex is Ice.ConnectTimeoutException) ||
                           (ex is Ice.SocketException) ||
                           (ex is Ice.DNSException))
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
                comm.destroy();
            }
            Console.Out.WriteLine("ok");
        }
        finally
        {
            if (IceInternal.AssemblyUtil.isWindows && isAdministrator)
            {
                store.Remove(caCert1);
                store.Remove(caCert2);
            }
            store.Close();
        }

        return factory;
    }
}
