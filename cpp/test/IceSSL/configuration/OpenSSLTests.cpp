// Copyright (c) ZeroC, Inc.

// Default to OpenSSL 3.0.0 compatibility (decimal mmnnpp format)
// Hide deprecated APIs unless the user explicitly wants them
#define OPENSSL_API_COMPAT 30000
#define OPENSSL_NO_DEPRECATED

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

const string password = "password"; // NOLINT(cert-err58-cpp)

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
    auto properties = Ice::createProperties();
    // Disable IPv6 for compatibility with Docker containers running on macOS.
    properties->setProperty("Ice.IPv6", "0");
    Ice::InitializationData initData{.properties = properties};
    Ice::CommunicatorPtr communicator = initialize(initData);
    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints(
        "ServerAdapter",
        helper->getTestEndpoint(20, "ssl"),
        serverAuthenticationOptions);
    auto obj = adapter->add<ServerPrx>(make_shared<ServerI>(communicator), Identity{.name = "server", .category = ""});
    adapter->activate();
    return communicator;
}

Ice::CommunicatorPtr
createClient(const optional<ClientAuthenticationOptions>& clientAuthenticationOptions)
{
    auto properties = Ice::createProperties();
    // Disable IPv6 for compatibility with Docker containers running on macOS.
    properties->setProperty("Ice.IPv6", "0");
    Ice::InitializationData initData{
        .properties = properties,
        .clientAuthenticationOptions = clientAuthenticationOptions.value_or(ClientAuthenticationOptions{})};
    if (IceInternal::isMinBuild())
    {
        initData.pluginFactories = {Ice::wsPluginFactory()};
    }
    return initialize(initData);
}

void
clientValidatesServerUsingCAFile(Test::TestHelper* helper, const string& testDir)
{
    cout << "client validates server certificate using a CAFile... " << flush;
    const string serverCertFile = testDir + "/ca1/server_cert.pem";
    const string serverKeyFile = testDir + "/ca1/server_key.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);

    const string clientCAFile = testDir + "/ca1/ca1_cert.pem";
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
clientValidatesServerUsingValidationCallback(Test::TestHelper* helper, const string& testDir)
{
    cout << "client validates server certificate using validation callback... " << flush;

    const string serverCertFile = testDir + "/ca1/server_cert.pem";
    const string serverKeyFile = testDir + "/ca1/server_key.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_server_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);

    // The server certificate is not trusted by the client CA, but the validation callback accepts the server
    // certificate.
    const string clientCAFile = testDir + "/ca2/ca2_cert.pem";
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
            .clientSSLContextSelectionCallback =
                [clientSSLContext](const string&)
            {
                SSL_CTX_up_ref(clientSSLContext);
                return clientSSLContext;
            },
            .serverCertificateValidationCallback = [](bool, X509_STORE_CTX*, const Ice::SSL::ConnectionInfoPtr&)
            { return true; }};
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
clientValidatesServerUsingSystemTrustedRootCertificates(Test::TestHelper*, const string&)
{
    cout << "client validates server using system trusted root certificates... " << flush;
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_default_verify_paths(clientSSLContext);
    Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{};
    Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

    Ice::ObjectPrx obj(
        clientCommunicator.communicator(),
        "Glacier2/router:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2");
    obj->ice_ping();
    cout << "ok" << endl;
}

void
clientRejectsServerUsingCAFile(Test::TestHelper* helper, const string& testDir)
{
    cout << "client rejects server certificate using a CAFile... " << flush;

    const string serverCertFile = testDir + "/ca1/server_cert.pem";
    const string serverKeyFile = testDir + "/ca1/server_key.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_server_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);

    // The CAs used by the client doesn't trust the server certificate.
    const string clientCAFile = testDir + "/ca2/ca2_cert.pem";
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
    const string serverCertFile = testDir + "/ca1/server_cert.pem";
    const string serverKeyFile = testDir + "/ca1/server_key.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_server_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);

    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
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
    const string serverCertFile = testDir + "/ca1/server_cert.pem";
    const string serverKeyFile = testDir + "/ca1/server_key.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);

    // The client trusted root certificates include the server certificate CA, but the validation callback
    // rejects the server certificate.
    const string clientCAFile = testDir + "/ca1/ca1_cert.pem";
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
    const string serverCertFile = testDir + "/ca1/server_cert.pem";
    const string serverKeyFile = testDir + "/ca1/server_key.pem";
    const string serverCAFile = testDir + "/ca1/ca1_cert.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(serverSSLContext, serverCAFile.c_str());

    const string clientCertFile = testDir + "/ca1/client_cert.pem";
    const string clientKeyFile = testDir + "/ca1/client_key.pem";
    const string clientCAFile = testDir + "/ca1/ca1_cert.pem";
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(clientSSLContext, clientCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(clientSSLContext, clientKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(clientSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(clientSSLContext, clientCAFile.c_str());

    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
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
serverValidatesClientUsingValidationCallback(Test::TestHelper* helper, const string& testDir)
{
    cout << "server validates client certificate using validation callback... " << flush;

    // The client certificate is not trusted by the server CA, but the validation callback accepts the client
    // certificate.
    const string serverCertFile = testDir + "/ca1/server_cert.pem";
    const string serverKeyFile = testDir + "/ca1/server_key.pem";
    const string serverCAFile = testDir + "/ca2/ca2_cert.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(serverSSLContext, serverCAFile.c_str());

    const string clientCertFile = testDir + "/ca1/client_cert.pem";
    const string clientKeyFile = testDir + "/ca1/client_key.pem";
    const string clientCAFile = testDir + "/ca1/ca1_cert.pem";
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(clientSSLContext, clientCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(clientSSLContext, clientKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(clientSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(clientSSLContext, clientCAFile.c_str());

    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverSSLContextSelectionCallback =
                [serverSSLContext](const string&)
            {
                SSL_CTX_up_ref(serverSSLContext);
                return serverSSLContext;
            },
            // Ensure that the client provides a certificate
            .sslNewSessionCallback = [](::SSL* ssl, const string&)
            { SSL_set_verify(ssl, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr); },
            .clientCertificateValidationCallback = [](bool, X509_STORE_CTX*, const Ice::SSL::ConnectionInfoPtr&)
            { return true; }};
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
serverRejectsClientUsingCAFile(Test::TestHelper* helper, const string& testDir)
{
    cout << "server rejects client certificate using a CAFile... " << flush;

    // The CAs used by the server doesn't trust the certificate used by the client.
    const string serverCertFile = testDir + "/ca1/server_cert.pem";
    const string serverKeyFile = testDir + "/ca1/server_key.pem";
    const string serverCAFile = testDir + "/ca2/ca2_cert.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(serverSSLContext, serverCAFile.c_str());

    const string clientCertFile = testDir + "/ca1/client_cert.pem";
    const string clientKeyFile = testDir + "/ca1/client_key.pem";
    const string clientCAFile = testDir + "/ca1/ca1_cert.pem";
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(clientSSLContext, clientCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(clientSSLContext, clientKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(clientSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(clientSSLContext, clientCAFile.c_str());

    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
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
    const string serverCertFile = testDir + "/ca1/server_cert.pem";
    const string serverKeyFile = testDir + "/ca1/server_key.pem";
    SSL_CTX* serverSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);

    const string clientCertFile = testDir + "/ca1/client_cert.pem";
    const string clientKeyFile = testDir + "/ca1/client_key.pem";
    const string clientCAFile = testDir + "/ca1/ca1_cert.pem";
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(clientSSLContext, clientCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(clientSSLContext, clientKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(clientSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(clientSSLContext, clientCAFile.c_str());

    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverSSLContextSelectionCallback =
                [serverSSLContext](const string&)
            {
                SSL_CTX_up_ref(serverSSLContext);
                return serverSSLContext;
            },
            .sslNewSessionCallback = [](::SSL* ssl, const string&)
            { SSL_set_verify(ssl, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr); }};
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

    // The server configured CAs trust the client certificate, but the installed validation callback explicitly
    // rejects the client certificate.
    const string serverCertFile = testDir + "/ca1/server_cert.pem";
    const string serverKeyFile = testDir + "/ca1/server_key.pem";
    const string serverCAFile = testDir + "/ca1/ca1_cert.pem";
    SSL_CTX_use_certificate_chain_file(serverSSLContext, serverCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(serverSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(serverSSLContext, serverCAFile.c_str());

    const string clientCertFile = testDir + "/ca1/client_cert.pem";
    const string clientKeyFile = testDir + "/ca1/client_key.pem";
    const string clientCAFile = testDir + "/ca1/ca1_cert.pem";
    SSL_CTX* clientSSLContext = SSL_CTX_new(TLS_method());
    SSL_CTX_use_certificate_chain_file(clientSSLContext, clientCertFile.c_str());
    SSL_CTX_use_PrivateKey_file(clientSSLContext, clientKeyFile.c_str(), SSL_FILETYPE_PEM);
    SSL_CTX_set_default_passwd_cb(clientSSLContext, passwordCallback);
    SSL_CTX_load_verify_file(clientSSLContext, clientCAFile.c_str());

    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
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
serverHotCertificateReload(Test::TestHelper* helper, const string& testDir)
{
    cout << "server hot certificate reload... " << flush;
    class ServerState final
    {
    public:
        ServerState(const string& serverCertFile, const string& serverKeyFile)
            : _serverSSLContext(SSL_CTX_new(TLS_method()))
        {
            SSL_CTX_use_certificate_chain_file(_serverSSLContext, serverCertFile.c_str());
            SSL_CTX_use_PrivateKey_file(_serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
            SSL_CTX_set_default_passwd_cb(_serverSSLContext, passwordCallback);
        }

        ~ServerState()
        {
            if (_serverSSLContext)
            {
                SSL_CTX_free(_serverSSLContext);
            }
        }

        [[nodiscard]] SSL_CTX* serverSSLContext() const { return _serverSSLContext; }

        void reloadSSLContext(const string& serverCertFile, const string& serverKeyFile)
        {
            if (_serverSSLContext)
            {
                SSL_CTX_free(_serverSSLContext);
            }
            _serverSSLContext = SSL_CTX_new(TLS_method());
            SSL_CTX_use_certificate_chain_file(_serverSSLContext, serverCertFile.c_str());
            SSL_CTX_use_PrivateKey_file(_serverSSLContext, serverKeyFile.c_str(), SSL_FILETYPE_PEM);
            SSL_CTX_set_default_passwd_cb(_serverSSLContext, passwordCallback);
        }

    private:
        SSL_CTX* _serverSSLContext = nullptr;
    };

    ServerState serverState(testDir + "/ca1/server_cert.pem", testDir + "/ca1/server_key.pem");

    const string clientCA1File = testDir + "/ca1/ca1_cert.pem";
    SSL_CTX* clientSSLContextCA1 = SSL_CTX_new(TLS_method());
    SSL_CTX_load_verify_file(clientSSLContextCA1, clientCA1File.c_str());

    const string clientCA2File = testDir + "/ca2/ca2_cert.pem";
    SSL_CTX* clientSSLContextCA2 = SSL_CTX_new(TLS_method());
    SSL_CTX_load_verify_file(clientSSLContextCA2, clientCA2File.c_str());

    try
    {
        Ice::SSL::ServerAuthenticationOptions serverAuthenticationOptions{
            .serverSSLContextSelectionCallback = [&serverState](const string&)
            {
                SSL_CTX* serverSSLContext = serverState.serverSSLContext();
                SSL_CTX_up_ref(serverSSLContext);
                return serverSSLContext;
            }};
        Ice::CommunicatorHolder serverCommunicator(createServer(serverAuthenticationOptions, helper));

        {
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .clientSSLContextSelectionCallback = [clientSSLContextCA1](const string&)
                {
                    SSL_CTX_up_ref(clientSSLContextCA1);
                    return clientSSLContextCA1;
                }};
            Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

            ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(20, "ssl"));
            obj->ice_ping();
        }

        {
            // CA2 is not accepted with the initial configuration
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .clientSSLContextSelectionCallback = [clientSSLContextCA2](const string&)
                {
                    SSL_CTX_up_ref(clientSSLContextCA2);
                    return clientSSLContextCA2;
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

        serverState.reloadSSLContext(testDir + "/ca2/server_cert.pem", testDir + "/ca2/server_key.pem");

        {
            // CA2 is accepted with the new configuration
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .clientSSLContextSelectionCallback = [clientSSLContextCA2](const string&)
                {
                    SSL_CTX_up_ref(clientSSLContextCA2);
                    return clientSSLContextCA2;
                }};
            Ice::CommunicatorHolder clientCommunicator(createClient(clientAuthenticationOptions));

            ServerPrx obj(clientCommunicator.communicator(), "server:" + helper->getTestEndpoint(20, "ssl"));
            obj->ice_ping();
        }

        {
            // CA1 is not accepted after reloading configuration
            Ice::SSL::ClientAuthenticationOptions clientAuthenticationOptions{
                .clientSSLContextSelectionCallback = [clientSSLContextCA1](const string&)
                {
                    SSL_CTX_up_ref(clientSSLContextCA1);
                    return clientSSLContextCA1;
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
    }
    catch (...)
    {
        SSL_CTX_free(clientSSLContextCA1);
        SSL_CTX_free(clientSSLContextCA2);
        throw;
    }
    SSL_CTX_free(clientSSLContextCA1);
    SSL_CTX_free(clientSSLContextCA2);
    cout << "ok" << endl;
}

void
allAuthenticationOptionsTests(Test::TestHelper* helper, const string& testDir)
{
    cout << "testing with OpenSSL native APIs..." << endl;

    clientValidatesServerUsingCAFile(helper, testDir);
    clientValidatesServerUsingValidationCallback(helper, testDir);
    clientValidatesServerUsingSystemTrustedRootCertificates(helper, testDir);
    clientRejectsServerUsingCAFile(helper, testDir);
    clientRejectsServerUsingDefaultSettings(helper, testDir);
    clientRejectsServerUsingValidationCallback(helper, testDir);

    serverValidatesClientUsingCAFile(helper, testDir);
    serverValidatesClientUsingValidationCallback(helper, testDir);
    serverRejectsClientUsingCAFile(helper, testDir);
    serverRejectsClientUsingDefaultSettings(helper, testDir);
    serverRejectsClientUsingValidationCallback(helper, testDir);

    serverHotCertificateReload(helper, testDir);
}

#endif
