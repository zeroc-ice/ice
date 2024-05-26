//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>

void
clientSSLContextSelectionCallbackExample()
{
    //! [clientSSLContextSelectionCallback]
    SSL_CTX* sslContext = SSL_CTX_new(TLS_method());
    // ...
    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
            .clientSSLContextSelectionCallback = [sslContext](const std::string&)
            {
                // Keep the SSLContext alive for the lifetime of the connection.
                SSL_CTX_up_ref(sslContext);
                return sslContext;
            }}};

    auto communicator = Ice::initialize(initData);
    // ...
    // Release ssl context when no longer needed
    SSL_CTX_free(sslContext);
    //! [clientSSLContextSelectionCallback]
}

void
clientSetNewSessionCallbackExample()
{
    //! [sslNewSessionCallback]
    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
            .sslNewSessionCallback = [](SSL* ssl, const std::string& host)
            {
                if (!SSL_set_cipher_list(ssl, "HIGH:!aNULL:!MD5"))
                {
                    // Handle error
                }
            }}};
    //! [sslNewSessionCallback]
}

void
serverCertificateValidationCallbackExample()
{
    //! [serverCertificateValidationCallback]
    auto initData = Ice::InitializationData{
        .clientAuthenticationOptions = Ice::SSL::ClientAuthenticationOptions{
            .serverCertificateValidationCallback =
                [](bool verified,
                   X509_STORE_CTX* ctx,
                   const Ice::SSL::ConnectionInfoPtr& info) { return verified; }}};
    //! [serverCertificateValidationCallback]
}
