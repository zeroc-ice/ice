// Copyright (c) ZeroC, Inc.

#include <Ice/Ice.h>

#if defined(ICE_USE_OPENSSL)

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
            .sslNewSessionCallback = [](SSL* ssl, const std::string&)
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
                [](bool verified, X509_STORE_CTX*, const Ice::SSL::ConnectionInfoPtr&)
            { return verified; }}};
    //! [serverCertificateValidationCallback]
}
#endif
