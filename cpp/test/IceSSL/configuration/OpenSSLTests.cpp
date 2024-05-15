//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "../../src/Ice/SSL/SecureTransportUtil.h"
#include "Ice/SSL/ClientAuthenticationOptions.h"
#include "Ice/SSL/ServerAuthenticationOptions.h"
#include "Test.h"
#include "TestHelper.h"
#include "TestI.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>

using namespace std;
using namespace Ice;
using namespace Ice::SSL;
using namespace Test;

#ifdef ICE_USE_OPENSSL

#    if defined(__GNUC__)
#        pragma GCC diagnostic push
#        pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#    endif

const string password = "password";

extern "C"
{
    int passwordCallback(char* buf, int size, int, void*)
    {
        size = std::min(size, static_cast<int>(password.size()));
        strncpy(buf, password.c_str(), size);
        buf[size - 1] = '\0';
        return size;
    }
}

Ice::CommunicatorPtr
createServer(ServerAuthenticationOptions serverAuthenticationOptions, TestHelper* helper)
{
    Ice::InitializationData initData;
    initData.properties = Ice::createProperties();
    // Disable IPv6 for compatibility with Docker containers running on macOS.
    initData.properties->setProperty("Ice.IPv6", "0");
    Ice::CommunicatorPtr communicator = initialize(initData);
    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints(
        "ServerAdapter",
        helper->getTestEndpoint(20, "ssl"),
        serverAuthenticationOptions);
    ServerPrx obj(adapter->add(make_shared<ServerI>(communicator), Identity{.name = "server", .category = ""}));
    adapter->activate();
    return communicator;
}

Ice::CommunicatorPtr
createClient(optional<ClientAuthenticationOptions> clientAuthenticationOptions)
{
    Ice::InitializationData initData;
    initData.properties = Ice::createProperties();
    // Disable IPv6 for compatibility with Docker containers running on macOS.
    initData.properties->setProperty("Ice.IPv6", "0");
    initData.clientAuthenticationOptions = clientAuthenticationOptions;
    return initialize(initData);
}

void
clientValidatesServerUsingCAFile(Test::TestHelper* helper, const string& testDir)
{
    cout << "client validates server certificate using a CAFile... " << flush;
    const string serverCertFile = testDir + "/../certs/s_rsa_ca1_pub.pem";
    const string serverKeyFile = testDir + "/../certs/s_rsa_ca1_priv.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);

    const string clientCAFile = testDir + "/../certs/cacert1.pem";
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_load_verify_file(clientSSLContext, clientCAFile.c_str());

    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverSSLContextSelectionCallback = [serverSSLContext](const string&)
            {
                SSL_CTX_up_ref(serverSSLContext);
                return serverSSLContext;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .clientSSLContextSelectionCallback = [clientSSLContext](const string&)
            {
                SSL_CTX_up_ref(clientSSLContext);
                return clientSSLContext;
            }};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(20, "ssl"));
        obj->ice_ping();
    }
    catch (...)
    {
        SSL_CTX_free(serverSSLContext);
        SSL_CTX_free(clientSSLContext);
        throw;
    }
    SSL_CTX_free(serverSSLContext);
    SSL_CTX_free(clientSSLContext);
    cout << "ok" << endl;
}

void
clientRejectsServerUsingCAFile(Test::TestHelper* helper, const string& testDir)
{
    cout << "client rejects server certificate using a CAFile... " << flush;

    const string serverCertFile = testDir + "/../certs/s_rsa_ca1_pub.pem";
    const string serverKeyFile = testDir + "/../certs/s_rsa_ca1_priv.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_server_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);

    // The CAs used by the client doesn't trust the server certificate.
    const string clientCAFile = testDir + "/../certs/cacert2.pem";
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_client_method());
    SSL_CTX_load_verify_file(clientSSLContext, clientCAFile.c_str());

    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverSSLContextSelectionCallback = [serverSSLContext](const string&)
            {
                SSL_CTX_up_ref(serverSSLContext);
                return serverSSLContext;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .clientSSLContextSelectionCallback = [clientSSLContext](const string&)
            {
                SSL_CTX_up_ref(clientSSLContext);
                return clientSSLContext;
            }};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));
        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(20, "ssl"));
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::SecurityException&)
        {
            // Expected
        }
    }
    catch (...)
    {
        SSL_CTX_free(serverSSLContext);
        SSL_CTX_free(clientSSLContext);
        throw;
    }
    SSL_CTX_free(serverSSLContext);
    SSL_CTX_free(clientSSLContext);
    cout << "ok" << endl;
}

void
clientRejectsServerUsingDefaultSettings(Test::TestHelper* helper, const string& testDir)
{
    cout << "client rejects server certificate using the default settings... " << flush;
    const string serverCertFile = testDir + "/../certs/s_rsa_ca1_pub.pem";
    const string serverKeyFile = testDir + "/../certs/s_rsa_ca1_priv.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_server_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);

    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverSSLContextSelectionCallback = [serverSSLContext](const string&)
            {
                SSL_CTX_up_ref(serverSSLContext);
                return serverSSLContext;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        // The client doesn't provide SSL authentication options. The system OpenSSL configuration would be used. The
        // system configuration doesn't trust the server certificate CA, and the certificate should be rejected.
        Ice::CommunicatorHolder clientCommunicator(createClient(nullopt));
        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(20, "ssl"));

        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::SecurityException&)
        {
            // Expected
        }
    }
    catch (...)
    {
        SSL_CTX_free(serverSSLContext);
        throw;
    }
    SSL_CTX_free(serverSSLContext);
    cout << "ok" << endl;
}

void
clientRejectsServerUsingValidationCallback(Test::TestHelper* helper, const string& testDir)
{
    cout << "client rejects server certificate using a validation callback... " << flush;
    const string serverCertFile = testDir + "/../certs/s_rsa_ca1_pub.pem";
    const string serverKeyFile = testDir + "/../certs/s_rsa_ca1_priv.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);

    // The client trusted the CA used by the server, but the client sets a validation callback that explicitly rejects
    // the server certificate.
    const string clientCAFile = testDir + "/../certs/cacert1.pem";
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_load_verify_file(clientSSLContext, clientCAFile.c_str());

    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverSSLContextSelectionCallback = [serverSSLContext](const string&)
            {
                SSL_CTX_up_ref(serverSSLContext);
                return serverSSLContext;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
            .clientSSLContextSelectionCallback =
                [clientSSLContext](const string&)
            {
                SSL_CTX_up_ref(clientSSLContext);
                return clientSSLContext;
            },
            .serverCertificateValidationCallback = [](bool, X509_STORE_CTX*, const Ice::SSL::ConnectionInfoPtr&)
            { return false; }};
        Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(20, "ssl"));

        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::SecurityException&)
        {
            // Expected
        }
    }
    catch (...)
    {
        SSL_CTX_free(serverSSLContext);
        SSL_CTX_free(clientSSLContext);
        throw;
    }
    SSL_CTX_free(serverSSLContext);
    SSL_CTX_free(clientSSLContext);
    cout << "ok" << endl;
}

void
serverValidatesClientUsingCAFile(Test::TestHelper* helper, const string& testDir)
{
    cout << "server validates client certificate using a CAFile... " << flush;

    // The CA used by the server trust the client certificate.
    const string serverCertFile = testDir + "/../certs/s_rsa_ca1_pub.pem";
    const string serverKeyFile = testDir + "/../certs/s_rsa_ca1_priv.pem";
    const string serverCAFile = testDir + "/../certs/cacert1.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(serverSSLContext, serverCAFile.c_str());

    const string clientCertFile = testDir + "/../certs/c_rsa_ca1_pub.pem";
    const string clientKeyFile = testDir + "/../certs/c_rsa_ca1_priv.pem";
    const string clientCAFile = testDir + "/../certs/cacert1.pem";
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(clientSSLContext, clientCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(clientSSLContext, clientKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(clientSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(clientSSLContext, clientCAFile.c_str());

    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverSSLContextSelectionCallback =
                [serverSSLContext](const string&)
            {
                SSL_CTX_up_ref(serverSSLContext);
                return serverSSLContext;
            },
            // Ensure that the client provides a certificate
            .sslNewSessionCallback = [](::SSL* ssl, const string&)
            { SSL_set_verify(ssl, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr); }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(createClient(Ice::SSL::ClientAuthenticationOptions{
            .clientSSLContextSelectionCallback = [clientSSLContext](const string&)
            {
                SSL_CTX_up_ref(clientSSLContext);
                return clientSSLContext;
            }}));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(20, "ssl"));
        obj->ice_ping();
    }
    catch (...)
    {
        SSL_CTX_free(serverSSLContext);
        SSL_CTX_free(clientSSLContext);
        throw;
    }
    SSL_CTX_free(serverSSLContext);
    SSL_CTX_free(clientSSLContext);
    cout << "ok" << endl;
}

void
serverRejectsClientUsingCAFile(Test::TestHelper* helper, const string& testDir)
{
    cout << "server rejects client certificate using a CAFile... " << flush;

    // The CAs used by the server doesn't trust the certificate used by the client.
    const string serverCertFile = testDir + "/../certs/s_rsa_ca1_pub.pem";
    const string serverKeyFile = testDir + "/../certs/s_rsa_ca1_priv.pem";
    const string serverCAFile = testDir + "/../certs/cacert2.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(serverSSLContext, serverCAFile.c_str());

    const string clientCertFile = testDir + "/../certs/c_rsa_ca1_pub.pem";
    const string clientKeyFile = testDir + "/../certs/c_rsa_ca1_priv.pem";
    const string clientCAFile = testDir + "/../certs/cacert1.pem";
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(clientSSLContext, clientCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(clientSSLContext, clientKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(clientSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(clientSSLContext, clientCAFile.c_str());

    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverSSLContextSelectionCallback =
                [serverSSLContext](const string&)
            {
                SSL_CTX_up_ref(serverSSLContext);
                return serverSSLContext;
            },
            // Ensure that the client provides a certificate
            .sslNewSessionCallback = [](::SSL* ssl, const string&)
            { SSL_set_verify(ssl, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr); }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(createClient(Ice::SSL::ClientAuthenticationOptions{
            .clientSSLContextSelectionCallback = [clientSSLContext](const string&)
            {
                SSL_CTX_up_ref(clientSSLContext);
                return clientSSLContext;
            }}));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(20, "ssl"));

        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
            // Expected
        }
    }
    catch (...)
    {
        SSL_CTX_free(serverSSLContext);
        SSL_CTX_free(clientSSLContext);
        throw;
    }
    SSL_CTX_free(serverSSLContext);
    SSL_CTX_free(clientSSLContext);
    cout << "ok" << endl;
}

void
serverRejectsClientUsingDefaultSettings(Test::TestHelper* helper, const string& testDir)
{
    cout << "server rejects client certificate using the default settings... " << flush;

    // The server doesn't configure any CAs, the system OpenSSL configuration would be used. The
    // system CAs don't trust the client certificate and the connection should be rejected.
    const string serverCertFile = testDir + "/../certs/s_rsa_ca1_pub.pem";
    const string serverKeyFile = testDir + "/../certs/s_rsa_ca1_priv.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);

    const string clientCertFile = testDir + "/../certs/c_rsa_ca1_pub.pem";
    const string clientKeyFile = testDir + "/../certs/c_rsa_ca1_priv.pem";
    const string clientCAFile = testDir + "/../certs/cacert1.pem";
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(clientSSLContext, clientCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(clientSSLContext, clientKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(clientSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(clientSSLContext, clientCAFile.c_str());

    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverSSLContextSelectionCallback =
                [serverSSLContext](const string&)
            {
                SSL_CTX_up_ref(serverSSLContext);
                return serverSSLContext;
            },
            .sslNewSessionCallback = [](::SSL* ssl, const string&)
            { SSL_set_verify(ssl, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr); }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(createClient(Ice::SSL::ClientAuthenticationOptions{
            .clientSSLContextSelectionCallback = [clientSSLContext](const string&)
            {
                SSL_CTX_up_ref(clientSSLContext);
                return clientSSLContext;
            }}));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(20, "ssl"));
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
            // Expected
        }
    }
    catch (...)
    {
        SSL_CTX_free(serverSSLContext);
        SSL_CTX_free(clientSSLContext);
        throw;
    }
    SSL_CTX_free(serverSSLContext);
    SSL_CTX_free(clientSSLContext);
    cout << "ok" << endl;
}

void
serverRejectsClientUsingValidationCallback(Test::TestHelper* helper, const string& testDir)
{
    cout << "server reject client certificate using a validation callback... " << flush;
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_method());

    // The server configured CAs trust the client certificate, but the installed validation callback explicitly rejects
    // the client certificate.
    const string serverCertFile = testDir + "/../certs/s_rsa_ca1_pub.pem";
    const string serverKeyFile = testDir + "/../certs/s_rsa_ca1_priv.pem";
    const string serverCAFile = testDir + "/../certs/cacert1.pem";
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(serverSSLContext, serverCAFile.c_str());

    const string clientCertFile = testDir + "/../certs/c_rsa_ca1_pub.pem";
    const string clientKeyFile = testDir + "/../certs/c_rsa_ca1_priv.pem";
    const string clientCAFile = testDir + "/../certs/cacert1.pem";
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(clientSSLContext, clientCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(clientSSLContext, clientKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(clientSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(clientSSLContext, clientCAFile.c_str());

    try
    {
        auto serverAuthenticationOptions = Ice::SSL::ServerAuthenticationOptions{
            .serverSSLContextSelectionCallback =
                [serverSSLContext](const string&)
            {
                SSL_CTX_up_ref(serverSSLContext);
                return serverSSLContext;
            },
            .sslNewSessionCallback = [](::SSL* ssl, const string&)
            { SSL_set_verify(ssl, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr); },
            .clientCertificateValidationCallback = [](bool, X509_STORE_CTX*, const Ice::SSL::ConnectionInfoPtr&)
            { return false; }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        Ice::CommunicatorHolder clientCommunicator(createClient(Ice::SSL::ClientAuthenticationOptions{
            .clientSSLContextSelectionCallback = [clientSSLContext](const string&)
            {
                SSL_CTX_up_ref(clientSSLContext);
                return clientSSLContext;
            }}));

        ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(20, "ssl"));
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
            // Expected
        }
    }
    catch (...)
    {
        SSL_CTX_free(serverSSLContext);
        SSL_CTX_free(clientSSLContext);
        throw;
    }
    SSL_CTX_free(serverSSLContext);
    SSL_CTX_free(clientSSLContext);
    cout << "ok" << endl;
}

void
allAuthenticationOptionsTests(Test::TestHelper* helper, const string& testDir)
{
    cerr << "testing with OpenSSL native APIs..." << endl;

    clientValidatesServerUsingCAFile(helper, testDir);
    clientRejectsServerUsingCAFile(helper, testDir);
    clientRejectsServerUsingDefaultSettings(helper, testDir);
    clientRejectsServerUsingValidationCallback(helper, testDir);

    serverValidatesClientUsingCAFile(helper, testDir);
    serverRejectsClientUsingCAFile(helper, testDir);
    serverRejectsClientUsingDefaultSettings(helper, testDir);
    serverRejectsClientUsingValidationCallback(helper, testDir);
}

#    if defined(__GNUC__)
#        pragma GCC diagnostic pop
#    endif

#endif
