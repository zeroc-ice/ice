// Copyright (c) ZeroC, Inc.

using System.Net.Security;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using Test;

public class AllTests : global::Test.AllTests
{
    private static X509Certificate2 createCertificate(string certPEM) =>
        new X509Certificate2(System.Text.Encoding.ASCII.GetBytes(certPEM));

    private static Ice.InitializationData
    createClientProps(Ice.Properties defaultProperties)
    {
        var result = new Ice.InitializationData();
        result.properties = new Ice.Properties();

        result.properties.setProperty("IceSSL.DefaultDir", defaultProperties.getIceProperty("IceSSL.DefaultDir"));
        result.properties.setProperty("Ice.Default.Host", defaultProperties.getIceProperty("Ice.Default.Host"));
        if (defaultProperties.getIceProperty("Ice.IPv6").Length > 0)
        {
            result.properties.setProperty("Ice.IPv6", defaultProperties.getIceProperty("Ice.IPv6"));
        }
        result.properties.setProperty("Ice.RetryIntervals", "-1");
        return result;
    }

    private static Dictionary<string, string>
    createServerProps(Ice.Properties defaultProperties)
    {
        var result = new Dictionary<string, string>
        {
            ["IceSSL.DefaultDir"] = defaultProperties.getIceProperty("IceSSL.DefaultDir"),
            ["Ice.Default.Host"] = defaultProperties.getIceProperty("Ice.Default.Host")
        };
        result["ServerAdapter.PublishedHost"] = result["Ice.Default.Host"];

        if (defaultProperties.getIceProperty("Ice.IPv6").Length > 0)
        {
            result["Ice.IPv6"] = defaultProperties.getIceProperty("Ice.IPv6");
        }
        return result;
    }

    private static Dictionary<string, string>
    createServerProps(Ice.Properties defaultProperties, string cert, string ca)
    {
        Dictionary<string, string> d = createServerProps(defaultProperties);
        if (cert.Length > 0)
        {
            d["IceSSL.CertFile"] = cert + ".p12";
        }
        if (ca.Length > 0)
        {
            d["IceSSL.CAs"] = ca + "_cert.pem";
        }
        d["IceSSL.Password"] = "password";
        return d;
    }

    private static Ice.InitializationData
    createClientProps(Ice.Properties defaultProperties, string cert, string ca)
    {
        Ice.InitializationData initData = createClientProps(defaultProperties);
        if (cert.Length > 0)
        {
            initData.properties.setProperty("IceSSL.CertFile", cert + ".p12");
        }
        if (ca.Length > 0)
        {
            initData.properties.setProperty("IceSSL.CAs", ca + "_cert.pem");
        }
        initData.properties.setProperty("IceSSL.Password", "password");
        return initData;
    }

    public static Test.ServerFactoryPrx allTests(Test.TestHelper helper, string defaultDir)
    {
        Ice.Communicator communicator = helper.communicator();
        string factoryRef = "factory:" + helper.getTestEndpoint(0, "tcp");

        Ice.ObjectPrx b = communicator.stringToProxy(factoryRef);
        test(b != null);
        Test.ServerFactoryPrx factory = Test.ServerFactoryPrxHelper.checkedCast(b);

        string defaultHost = communicator.getProperties().getIceProperty("Ice.Default.Host");

        Ice.Properties defaultProperties = communicator.getProperties();
        defaultProperties.setProperty("IceSSL.DefaultDir", defaultDir);
        defaultProperties.setProperty("Ice.Default.Host", defaultHost);

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
        string caCert1File = defaultDir + "/ca1/ca1_cert.pem";
        string caCert2File = defaultDir + "/ca2/ca2_cert.pem";
        var caCert1 = new X509Certificate2(caCert1File);
        var caCert2 = new X509Certificate2(caCert2File);

        test(Enumerable.SequenceEqual(createCertificate(File.ReadAllText(caCert1File)).RawData, caCert1.RawData));
        test(Enumerable.SequenceEqual(createCertificate(File.ReadAllText(caCert2File)).RawData, caCert2.RawData));

        var store = new X509Store(StoreName.AuthRoot, StoreLocation.LocalMachine);
        bool isAdministrator = false;
        if (Ice.Internal.AssemblyUtil.isWindows)
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

        Ice.InitializationData initData;
        Dictionary<string, string> d;
        try
        {
            var args = new string[] { };

            Console.Out.Write("testing certificate without password... ");
            Console.Out.Flush();
            {
                initData = createClientProps(defaultProperties, "ca1/client_password_less", "ca1/ca1");
                initData.properties.setProperty("IceSSL.Password", "");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca1/server_password_less", "ca1/ca1");
                d["IceSSL.Password"] = "";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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

            Console.Out.Write("testing certificate verification... ");
            Console.Out.Flush();
            {
                //
                // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
                // and it doesn't trust the server certificate.
                //
                initData = createClientProps(defaultProperties, "", "");
                initData.properties.setProperty("IceSSL.VerifyPeer", "0");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca1/server", "");
                d["IceSSL.VerifyPeer"] = "0";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.noCert();
                    test(false);
                }
                catch (Ice.SecurityException)
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
                // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
                // but it still verifies the server's.
                //
                initData = createClientProps(defaultProperties, "", "ca1/ca1");
                initData.properties.setProperty("IceSSL.VerifyPeer", "0");
                comm = Ice.Util.initialize(ref args, initData);
                fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca1/server", "");
                d["IceSSL.VerifyPeer"] = "0";
                server = fact.createServer(d);
                try
                {
                    server.noCert();
                    test(((Ice.SSL.ConnectionInfo)server.ice_getConnection().getInfo()).verified);
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
                initData = createClientProps(defaultProperties, "", "ca1/ca1");
                comm = Ice.Util.initialize(ref args, initData);
                fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca1/server", "");
                d["IceSSL.VerifyPeer"] = "1";
                server = fact.createServer(d);
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
                d = createServerProps(defaultProperties, "ca1/server", "");
                d["IceSSL.VerifyPeer"] = "2";
                server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.ConnectionLostException)
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

                // Test IceSSL.VerifyPeer=1. Client has a certificate.
                //
                // Provide "ca1/ca1" to the client to verify the server certificate (without this the client connection
                // wouldn't be able to provide the certificate chain).
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                comm = Ice.Util.initialize(ref args, initData);
                fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.VerifyPeer"] = "1";
                server = fact.createServer(d);
                try
                {
                    var clientCert =
                        new X509Certificate2(defaultDir + "/ca1/client.p12", "password");
                    server.checkCert(clientCert.Subject, clientCert.Issuer);

                    var serverCert =
                        new X509Certificate2(defaultDir + "/ca1/server.p12", "password");
                    var caCert = new X509Certificate2(defaultDir + "/ca1/ca1_cert.pem");

                    var info = (Ice.SSL.ConnectionInfo)server.ice_getConnection().getInfo();
                    test(info.certs.Length == 1);
                    test(info.verified);

                    test(serverCert.Equals(info.certs[0]));
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);

                //
                // Test IceSSL.VerifyPeer=2. Client has a certificate.
                //
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.VerifyPeer"] = "2";
                server = fact.createServer(d);
                try
                {
                    var clientCert = new X509Certificate2(defaultDir + "/ca1/client.p12", "password");
                    server.checkCert(clientCert.Subject, clientCert.Issuer);
                }
                catch (Exception ex)
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
                initData = createClientProps(defaultProperties, "", "");
                comm = Ice.Util.initialize(ref args, initData);
                fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca1/server", "");
                d["IceSSL.VerifyPeer"] = "0";
                server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.SecurityException)
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
                initData = createClientProps(defaultProperties, "ca2/client", "");
                initData.properties.setProperty("IceSSL.VerifyPeer", "0");
                comm = Ice.Util.initialize(ref args, initData);
                fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca1/server", "");
                d["IceSSL.VerifyPeer"] = "1";
                server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.SecurityException)
                {
                    // Expected.
                }
                catch (Ice.ConnectionLostException)
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
                // This should succeed because the self signed certificate used by the server is
                // trusted.
                //
                initData = createClientProps(defaultProperties, "", "ca2/ca2");
                comm = Ice.Util.initialize(ref args, initData);
                fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca2/ca2", "");
                d["IceSSL.VerifyPeer"] = "0";
                server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                }
                catch (Ice.LocalException ex)
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
                initData = createClientProps(defaultProperties);
                comm = Ice.Util.initialize(ref args, initData);
                fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca2/ca2", "");
                d["IceSSL.VerifyPeer"] = "0";
                server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.SecurityException)
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
                // Verify that IceSSL.CheckCertName has no effect in a server.
                //
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                comm = Ice.Util.initialize(ref args, initData);
                fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.CheckCertName"] = "1";
                server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                }
                catch (Ice.LocalException ex)
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
                if (defaultHost == "127.0.0.1")
                {
                    //
                    // Test using localhost as target host
                    //
                    Ice.Properties props = defaultProperties.Clone();
                    props.setProperty("Ice.Default.Host", "localhost");

                    //
                    // Target host matches the certificate DNS altName
                    //
                    {
                        initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                        initData.properties.setProperty("IceSSL.CheckCertName", "1");
                        comm = Ice.Util.initialize(ref args, initData);

                        fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                        test(fact != null);
                        d = createServerProps(props, "ca1/server_cn1", "ca1/ca1");
                        d["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(d);
                        try
                        {
                            server.ice_ping();
                        }
                        catch (Ice.LocalException ex)
                        {
                            // macOS catalina or greater does not check the certificate common name
                            if (!Ice.Internal.AssemblyUtil.isMacOS)
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
                        initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                        initData.properties.setProperty("IceSSL.CheckCertName", "1");
                        comm = Ice.Util.initialize(ref args, initData);

                        fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                        test(fact != null);
                        d = createServerProps(props, "ca1/server_cn2", "ca1/ca1");
                        d["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(d);
                        try
                        {
                            server.ice_ping();
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
                    // Target host matches the certificate Common Name and the certificate does not
                    // include a DNS altName
                    //
                    {
                        initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                        initData.properties.setProperty("IceSSL.CheckCertName", "1");
                        comm = Ice.Util.initialize(ref args, initData);

                        fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                        test(fact != null);
                        d = createServerProps(props, "ca1/server_cn3", "ca1/ca1");
                        d["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(d);
                        try
                        {
                            server.ice_ping();
                        }
                        catch (Ice.LocalException)
                        {
                            // macOS >= Catalina requires a DNS altName. DNS name as the Common Name is not trusted
                            test(Ice.Internal.AssemblyUtil.isMacOS);
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }
                    //
                    // Target host does not match the certificate Common Name and the certificate does not
                    // include a DNS altName
                    //
                    {
                        initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                        initData.properties.setProperty("IceSSL.CheckCertName", "1");
                        comm = Ice.Util.initialize(ref args, initData);

                        fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                        test(fact != null);
                        d = createServerProps(props, "ca1/server_cn4", "ca1/ca1");
                        d["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(d);
                        try
                        {
                            server.ice_ping();
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
                    // Target host matches the certificate Common Name and the certificate has
                    // a DNS altName that does not matches the target host
                    //
                    {
                        initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                        initData.properties.setProperty("IceSSL.CheckCertName", "1");
                        comm = Ice.Util.initialize(ref args, initData);

                        fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                        test(fact != null);
                        d = createServerProps(props, "ca1/server_cn5", "ca1/ca1");
                        d["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(d);
                        try
                        {
                            server.ice_ping();
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

                    // Disabled for compatibility with older Windows versions.
                    //
                    // Target host matches the certificate IP altName
                    {
                        initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                        initData.properties.setProperty("IceSSL.CheckCertName", "1");
                        comm = Ice.Util.initialize(ref args, initData);

                        fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                        test(fact != null);
                        d = createServerProps(defaultProperties, "ca1/server_cn6", "ca1/ca1");
                        d["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(d);
                        try
                        {
                            server.ice_ping();
                        }
                        catch (Ice.LocalException)
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
                        initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                        initData.properties.setProperty("IceSSL.CheckCertName", "1");
                        comm = Ice.Util.initialize(ref args, initData);

                        fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                        test(fact != null);
                        d = createServerProps(defaultProperties, "ca1/server_cn7", "ca1/ca1");
                        d["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(d);
                        try
                        {
                            server.ice_ping();
                            test(false);
                        }
                        catch (Ice.SecurityException)
                        {
                            // Expected
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }

                    // Target host is an IP address that matches the CN and the certificate doesn't
                    // include an IP altName.
                    {
                        initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                        initData.properties.setProperty("IceSSL.CheckCertName", "1");
                        comm = Ice.Util.initialize(ref args, initData);

                        fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                        test(fact != null);
                        d = createServerProps(defaultProperties, "ca1/server_cn8", "ca1/ca1");
                        d["IceSSL.CheckCertName"] = "1";
                        server = fact.createServer(d);
                        try
                        {
                            server.ice_ping();
                        }
                        catch (Ice.SecurityException ex)
                        {
                            // macOS catalina or greater does not check the certificate common name
                            if (!Ice.Internal.AssemblyUtil.isMacOS)
                            {
                                Console.WriteLine(ex.ToString());
                                test(false);
                            }
                        }
                        fact.destroyServer(server);
                        comm.destroy();
                    }
                }
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing expired certificates... ");
            Console.Out.Flush();
            {
                // This should fail because the server's certificate is expired.
                initData = createClientProps(defaultProperties, "ca1/client", "ca5/ca5");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca5/server_expired", "ca1/ca1");
                d["IceSSL.VerifyPeer"] = "2";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.SecurityException)
                {
                    // Expected.
                }
                catch (Ice.LocalException ex)
                {
                    Console.Out.Write(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();

                // This should fail because the client's certificate is expired.
                initData = createClientProps(defaultProperties, "ca5/client_expired", "ca1/ca1");
                comm = Ice.Util.initialize(ref args, initData);
                fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca1/server", "ca5/ca5");
                d["IceSSL.VerifyPeer"] = "2";
                server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.ConnectionLostException)
                {
                    // Expected.
                }
                catch (Ice.LocalException ex)
                {
                    Console.Out.Write(ex.ToString());
                    test(false);
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            Console.Out.WriteLine("ok");

            if (Ice.Internal.AssemblyUtil.isWindows && isAdministrator)
            {
                // LocalMachine certificate store is not supported on non Windows platforms.
                Console.Out.Write("testing multiple CA certificates... ");
                Console.Out.Flush();
                {
                    initData = createClientProps(defaultProperties, "ca1/client", "");
                    initData.properties.setProperty("IceSSL.UsePlatformCAs", "1");
                    Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
                    Test.ServerFactoryPrx fact =
                        Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(defaultProperties, "ca2/ca2", "");
                    d["IceSSL.VerifyPeer"] = "2";
                    d["IceSSL.UsePlatformCAs"] = "1";
                    store.Add(caCert1);
                    store.Add(caCert2);
                    Test.ServerPrx server = fact.createServer(d);
                    try
                    {
                        server.ice_ping();
                    }
                    catch (Ice.LocalException ex)
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca_all");
                Ice.Communicator comm = Ice.Util.initialize(initData);
                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca2/server", "ca_all");
                d["IceSSL.VerifyPeer"] = "2";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "");
                initData.properties.setProperty("IceSSL.CAs", "ca1/ca1_cert.der");
                Ice.Communicator comm = Ice.Util.initialize(initData);
                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                test(fact != null);
                d = createServerProps(defaultProperties, "ca1/server", "");
                d["IceSSL.VerifyPeer"] = "2";
                d["IceSSL.CAs"] = "ca1/ca1_cert.der";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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

            Console.Out.Write("testing IceSSL.TrustOnly... ");
            Console.Out.Flush();
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly",
                    "C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.server");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly",
                    "!C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.server");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly",
                    "C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.server");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly"] =
                    "C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.client";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly"] =
                    "!C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.client";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly", "CN=ca1.server");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly", "!CN=ca1.server");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly"] = "CN=ca1.client";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly"] = "!CN=ca1.client";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly", "CN=ca1.client");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly"] = "CN=ca1.server";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly", "C=Canada,CN=ca1.server");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly", "!C=Canada,CN=ca1.server");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly", "C=Canada;CN=ca1.server");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly", "!C=Canada;!CN=ca1.server");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly", "!CN=ca1.server_x"); // Should not match "ca1.server"
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly"] = "!CN=ca1.client_x"; // Should not match "ca1.client"
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                // Rejection takes precedence (client).
                //
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly", "ST=Florida;!CN=ca1.server;C=US");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                //
                // Rejection takes precedence (server).
                //
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly"] = "C=US;!CN=ca1.client;ST=Florida";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing IceSSL.TrustOnly.Client... ");
            Console.Out.Flush();
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly.Client",
                    "C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.server");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                // Should have no effect.
                d["IceSSL.TrustOnly.Client"] =
                    "C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.server";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly.Client",
                    "!C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.server");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                // Should have no effect.
                d["IceSSL.TrustOnly.Client"] = "!CN=ca1.client";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly.Client", "CN=ca1.client");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                initData.properties.setProperty("IceSSL.TrustOnly.Client", "!CN=ca1.client");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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

            Console.Out.Write("testing IceSSL.TrustOnly.Server... ");
            Console.Out.Flush();
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                // Should have no effect.
                initData.properties.setProperty("IceSSL.TrustOnly.Server",
                    "C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.client");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly.Server"] =
                    "C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.client";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly.Server"] =
                    "!C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.client";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                // Should have no effect.
                initData.properties.setProperty("IceSSL.TrustOnly.Server", "!CN=ca1.server");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly.Server"] = "CN=ca1.server";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly.Server"] = "!CN=ca1.client";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly.Server"] = "CN=bogus";
                d["IceSSL.TrustOnly.Server.ServerAdapter"] =
                    "C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.client";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly.Server.ServerAdapter"] =
                    "!C=US, ST=Florida, O=ZeroC,OU=Ice test infrastructure, emailAddress=info@zeroc.com, CN=ca1.client";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly.Server.ServerAdapter"] = "CN=bogus";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
                    test(false);
                }
                catch (Ice.LocalException)
                {
                }
                fact.destroyServer(server);
                comm.destroy();
            }
            {
                initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1");
                Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1");
                d["IceSSL.TrustOnly.Server.ServerAdapter"] = "!CN=bogus";
                Test.ServerPrx server = fact.createServer(d);
                try
                {
                    server.ice_ping();
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

            if (!Ice.Internal.AssemblyUtil.isMacOS)
            {
                Console.Out.Write("testing IceSSL.FindCerts properties... ");
                Console.Out.Flush();
                string[] clientFindCertProperties =
                [
                    "SUBJECTDN:'CN=ca1.client, C=US, S=Florida, L=Jupiter, O=ZeroC, OU=Ice test infrastructure, E=info@zeroc.com'",
                    "ISSUER:ca1 SUBJECT:ca1.client SERIAL:'10 01'",
                    "ISSUERDN:'C=US, S=Florida, L=Jupiter, O=ZeroC, OU=Ice test infrastructure, CN=ca1, E=info@zeroc.com' SUBJECT:ca1.client",
                    "THUMBPRINT:'7E 28 35 0E 67 B7 39 C4 28 AF CC B8 8E EF AA B5 55 33 43 25'",
                    "SUBJECTKEYID:'35 F5 EB 10 49 80 8D 0B 3D 25 67 AB 43 1A EC EB 3D CE 95 28'"
                ];

                string[] serverFindCertProperties =
                [
                    "SUBJECTDN:'CN=ca1.server, C=US, S=Florida, L=Jupiter, O=ZeroC, OU=Ice test infrastructure, E=info@zeroc.com'",
                    "ISSUER:ca1 SUBJECT:ca1.server SERIAL:'10 0A'",
                    "ISSUERDN:'C=US, S=Florida, L=Jupiter, O=ZeroC, OU=Ice test infrastructure, CN=ca1, E=info@zeroc.com' SUBJECT:ca1.server",
                    "THUMBPRINT:'B0 5B 31 83 9D AC A6 D1 3D B3 BF EC 6F E5 CB B8 27 F4 C5 E5'",
                    "SUBJECTKEYID:'21 27 14 AD E7 AC 9A 3D A7 27 24 EA 33 E7 07 2F 97 17 B1 AF'"
                ];

                string[] failFindCertProperties =
                [
                    "nolabel",
                    "unknownlabel:foo",
                    "LABEL:",
                    "SUBJECTDN:'CN = Client, E = infox@zeroc.com, OU = Ice, O = ZeroC, S = Florida, C = US'",
                    "ISSUER:'ZeroC, Inc.' SUBJECT:Client SERIAL:'02 02'",
                    "ISSUERDN:'E=info@zeroc.com, CN=ZeroC Test CA 1, OU=Ice test infrastructure, O=ZeroC," +
                        " L=Jupiter, S=Florida, C=ES' SUBJECT:Client",
                    "THUMBPRINT:'27 e0 18 c9 23 12 6c f0 5c da fa 36 5a 4c 63 5a e2 53 07 ff'",
                    "SUBJECTKEYID:'a6 42 aa 17 04 41 86 56 67 e4 04 64 59 34 30 c7 4c 6b ef ff'"
                ];

                string[] certificates = ["/ca1/server.p12", "/ca1/client.p12"];

                var certStore = new X509Store("My", StoreLocation.CurrentUser);
                certStore.Open(OpenFlags.ReadWrite);
                var storageFlags = X509KeyStorageFlags.DefaultKeySet;
                try
                {
                    foreach (string cert in certificates)
                    {
                        certStore.Add(new X509Certificate2(defaultDir + cert, "password", storageFlags));
                    }

                    for (int i = 0; i < clientFindCertProperties.Length; ++i)
                    {
                        initData = createClientProps(defaultProperties, "", "ca1/ca1");
                        initData.properties.setProperty("IceSSL.CertStore", "My");
                        initData.properties.setProperty("IceSSL.CertStoreLocation", "CurrentUser");
                        initData.properties.setProperty("IceSSL.FindCert", clientFindCertProperties[i]);

                        // Use TrustOnly to ensure the peer has pick the expected certificate.
                        initData.properties.setProperty("IceSSL.TrustOnly", "CN=ca1.server");
                        Ice.Communicator comm = Ice.Util.initialize(ref args, initData);

                        Test.ServerFactoryPrx fact =
                            Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                        d = createServerProps(defaultProperties, "", "ca1/ca1");
                        d["IceSSL.FindCert"] = serverFindCertProperties[i];
                        // Use TrustOnly to ensure the peer has pick the expected certificate.
                        d["IceSSL.TrustOnly"] = "CN=ca1.client";

                        Test.ServerPrx server = fact.createServer(d);
                        try
                        {
                            server.ice_ping();
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
                            initData = createClientProps(defaultProperties);
                            initData.properties.setProperty("IceSSL.FindCert", s);
                            Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
                            test(false);
                        }
                        catch (Ice.InitializationException)
                        {
                            // Expected
                        }
                        catch (Ice.LocalException ex)
                        {
                            Console.WriteLine(ex.ToString());
                            test(false);
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
                        initData = createClientProps(defaultProperties);
                        initData.properties.setProperty("IceSSL.FindCert", s);
                        Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
                        test(false);
                    }
                    catch (Ice.InitializationException)
                    {
                        // Expected
                    }
                    catch (Ice.LocalException ex)
                    {
                        Console.WriteLine(ex.ToString());
                        test(false);
                    }
                }
                Console.Out.WriteLine("ok");
            }

            Console.Out.Write("testing system CAs... ");
            Console.Out.Flush();
            {
                //
                // Retry a few times in case there are connectivity problems with demo.zeroc.com.
                //
                const int retryMax = 5;
                const int retryDelay = 1000;
                int retryCount = 0;

                initData = createClientProps(defaultProperties);
                initData.properties.setProperty("IceSSL.DefaultDir", "");
                Ice.Communicator comm = Ice.Util.initialize(initData);
                Ice.ObjectPrx p = comm.stringToProxy("dummy:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2");
                while (true)
                {
                    try
                    {
                        p.ice_ping();
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
                initData = createClientProps(defaultProperties);
                initData.properties.setProperty("IceSSL.DefaultDir", "");
                initData.properties.setProperty("IceSSL.UsePlatformCAs", "1");
                comm = Ice.Util.initialize(initData);
                p = comm.stringToProxy("dummy:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2");
                while (true)
                {
                    try
                    {
                        var info = (Ice.SSL.ConnectionInfo)p.ice_getConnection().getInfo().underlying;
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
            if (Ice.Internal.AssemblyUtil.isWindows && isAdministrator)
            {
                store.Remove(caCert1);
                store.Remove(caCert2);
            }
            store.Close();
        }

        return factory;
    }

    internal class Pingable : PingableDisp_
    {
        public override void ping(Ice.Current current)
        {
        }
    }
}
