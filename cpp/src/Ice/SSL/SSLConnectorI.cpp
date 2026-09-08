// Copyright (c) ZeroC, Inc.

#include "SSLConnectorI.h"
#include "../TargetCompare.h"
#include "Ice/SSL/ClientAuthenticationOptions.h"
#include "SSLEngine.h"
#include "SSLInstance.h"

#if defined(ICE_USE_SCHANNEL)
#    include "SchannelEngine.h"
#    include "SchannelTransceiverI.h"
using namespace Ice::SSL::Schannel;
#elif defined(ICE_USE_APPLE_SSL)
#    include "../apple/NetworkFrameworkConnector.h"
#    include "../apple/NetworkFrameworkTransceiver.h"
#    include "Ice/Connection.h"
#    include "Ice/SSL/ConnectionInfo.h"
#    include <Network/Network.h>
#elif defined(ICE_USE_OPENSSL)
#    include "OpenSSLEngine.h"
#    include "OpenSSLTransceiverI.h"
using namespace Ice::SSL::OpenSSL;
#endif

using namespace std;
using namespace Ice;
using namespace Ice::SSL;

IceInternal::TransceiverPtr
Ice::SSL::ConnectorI::connect()
{
    optional<Ice::SSL::ClientAuthenticationOptions> clientAuthenticationOptions =
        _instance->engine()->getInitializationData().clientAuthenticationOptions;
    if (!clientAuthenticationOptions)
    {
        clientAuthenticationOptions = _instance->engine()->createClientAuthenticationOptions(_host);
    }
    assert(clientAuthenticationOptions);

#if defined(ICE_USE_NETWORK_FRAMEWORK)
    //
    // For Network.framework, TLS is configured in the connection parameters rather than
    // as a transceiver wrapper. Create a TLS-enabled nw_connection_t directly.
    //
    auto* nfConnector = dynamic_cast<const IceInternal::NetworkFrameworkConnector*>(_delegate.get());
    assert(nfConnector);

    nw_endpoint_t endpoint = nw_endpoint_create_host(nfConnector->host().c_str(), to_string(nfConnector->port()).c_str());
    if (!endpoint)
    {
        throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
    }

    auto authOptions = *clientAuthenticationOptions;
    string host = _host;

    // Shared flag to track whether our local verify block rejected the server's certificate.
    // This allows the transceiver to distinguish "we rejected the peer" (SecurityException)
    // from "the peer rejected us" (ConnectionLostException).
    auto localVerifyRejected = make_shared<atomic<bool>>(false);

    nw_parameters_t parameters = nw_parameters_create_secure_tcp(
        ^(nw_protocol_options_t tlsOptions) {
            sec_protocol_options_t secOptions = nw_tls_copy_sec_protocol_options(tlsOptions);

            // Set TLS server name for SNI.
            if (!host.empty())
            {
                sec_protocol_options_set_tls_server_name(secOptions, host.c_str());
            }

            // Set client certificate identity, including any intermediate certificates.
            if (authOptions.clientCertificateSelectionCallback)
            {
                CFArrayRef certs = authOptions.clientCertificateSelectionCallback(host);
                if (certs && CFArrayGetCount(certs) > 0)
                {
                    SecIdentityRef identity = (SecIdentityRef)CFArrayGetValueAtIndex(certs, 0);
                    sec_identity_t secIdentity = nullptr;
                    CFIndex count = CFArrayGetCount(certs);
                    if (count > 1)
                    {
                        CFMutableArrayRef intermediateCerts =
                            CFArrayCreateMutable(kCFAllocatorDefault, count - 1, &kCFTypeArrayCallBacks);
                        for (CFIndex i = 1; i < count; ++i)
                        {
                            CFArrayAppendValue(intermediateCerts, CFArrayGetValueAtIndex(certs, i));
                        }
                        secIdentity = sec_identity_create_with_certificates(identity, intermediateCerts);
                        CFRelease(intermediateCerts);
                    }
                    else
                    {
                        secIdentity = sec_identity_create(identity);
                    }
                    if (secIdentity)
                    {
                        sec_protocol_options_set_local_identity(secOptions, secIdentity);
                        sec_release(secIdentity);
                    }
                    CFRelease(certs);
                }
            }

            // Set certificate verification block.
            dispatch_queue_t verifyQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
            CFArrayRef trustedRoots = authOptions.trustedRootCertificates;
            auto validationCallback = authOptions.serverCertificateValidationCallback;
            sec_protocol_options_set_verify_block(
                secOptions,
                ^(sec_protocol_metadata_t metadata, sec_trust_t trustRef, sec_protocol_verify_complete_t complete) {
                    // NF's sec_trust_copy_ref only provides the leaf certificate. We need
                    // the full peer certificate chain (including intermediates) to build a
                    // proper trust for evaluation. Extract it from the protocol metadata.
                    CFMutableArrayRef peerCerts =
                        CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
                    sec_protocol_metadata_access_peer_certificate_chain(
                        metadata,
                        ^(sec_certificate_t cert) {
                            SecCertificateRef secCert = sec_certificate_copy_ref(cert);
                            CFArrayAppendValue(peerCerts, secCert);
                            CFRelease(secCert);
                        });

                    // Create a new trust with the full certificate chain.
                    SecPolicyRef policy;
                    if (!host.empty())
                    {
                        CFStringRef hostRef =
                            CFStringCreateWithCString(kCFAllocatorDefault, host.c_str(), kCFStringEncodingUTF8);
                        policy = SecPolicyCreateSSL(true, hostRef);
                        CFRelease(hostRef);
                    }
                    else
                    {
                        policy = SecPolicyCreateBasicX509();
                    }

                    SecTrustRef trust;
                    OSStatus status = SecTrustCreateWithCertificates(peerCerts, policy, &trust);
                    CFRelease(policy);
                    CFRelease(peerCerts);

                    if (status != errSecSuccess || !trust)
                    {
                        localVerifyRejected->store(true);
                        complete(false);
                        return;
                    }

                    // Set trusted root certificates if provided.
                    if (trustedRoots)
                    {
                        SecTrustSetAnchorCertificates(trust, trustedRoots);
                        SecTrustSetAnchorCertificatesOnly(trust, true);
                    }

                    if (validationCallback)
                    {
                        try
                        {
                            // Construct a minimal ConnectionInfo with the peer certificate so
                            // TrustOnly and other DN-based checks can work during the handshake.
                            // SecTrustGetCertificateAtIndex returns an unretained ref valid while
                            // the trust is alive.
                            SecCertificateRef peerCert = nullptr;
                            if (SecTrustGetCertificateCount(trust) > 0)
                            {
                                peerCert = SecTrustGetCertificateAtIndex(trust, 0);
                                if (peerCert)
                                {
                                    CFRetain(peerCert); // AppleConnectionInfo releases it.
                                }
                            }
                            auto underlying = make_shared<Ice::TCPConnectionInfo>(
                                false, "", "", "", 0, "", 0, 0, 0);
                            auto info = make_shared<Ice::SSL::AppleConnectionInfo>(
                                underlying, peerCert);
                            bool valid = validationCallback(trust, info);
                            if (!valid)
                            {
                                localVerifyRejected->store(true);
                            }
                            complete(valid);
                        }
                        catch (const std::exception&)
                        {
                            localVerifyRejected->store(true);
                            complete(false);
                        }
                        catch (...)
                        {
                            localVerifyRejected->store(true);
                            complete(false);
                        }
                    }
                    else
                    {
                        // Default validation using SecTrust.
                        CFErrorRef error = nullptr;
                        bool valid = SecTrustEvaluateWithError(trust, &error);
                        if (error)
                        {
                            CFRelease(error);
                        }
                        if (!valid)
                        {
                            localVerifyRejected->store(true);
                        }
                        complete(valid);
                    }
                    CFRelease(trust);
                },
                verifyQueue);

            // Invoke the application's callback last so that the settings it applies take precedence over the
            // configuration above.
            if (authOptions.sslNewSessionCallback)
            {
                authOptions.sslNewSessionCallback(secOptions, host);
            }

            nw_release(secOptions); // nw_tls_copy_sec_protocol_options returns a retained object.
        },
        NW_PARAMETERS_DEFAULT_CONFIGURATION);

    if (!parameters)
    {
        nw_release(endpoint);
        throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
    }

    nw_connection_t connection = nw_connection_create(endpoint, parameters);
    nw_release(endpoint);
    nw_release(parameters);

    if (!connection)
    {
        throw Ice::ConnectFailedException(__FILE__, __LINE__, 0);
    }

    try
    {
        IceInternal::ProtocolInstancePtr protocolInstance = _instance;
        auto transceiver = make_shared<IceInternal::NetworkFrameworkTransceiver>(
            protocolInstance, connection, true /* secure */);
        transceiver->setLocalVerifyRejected(localVerifyRejected);
        SSLEnginePtr engine = _instance->engine();
        transceiver->setPeerVerifier(
            [engine](const ConnectionInfoPtr& info) { engine->verifyPeer(info); },
            false,
            "");
        nw_release(connection);
        return transceiver;
    }
    catch (...)
    {
        nw_release(connection);
        throw;
    }
#else
    return make_shared<TransceiverI>(_instance, _delegate->connect(), _host, *clientAuthenticationOptions);
#endif
}

int16_t
Ice::SSL::ConnectorI::type() const
{
    return _delegate->type();
}

string
Ice::SSL::ConnectorI::toString() const
{
    return _delegate->toString();
}

bool
Ice::SSL::ConnectorI::operator==(const IceInternal::Connector& r) const
{
    const auto* p = dynamic_cast<const ConnectorI*>(&r);
    if (!p)
    {
        return false;
    }

    if (this == p)
    {
        return true;
    }

    return Ice::targetEqualTo(_delegate, p->_delegate);
}

bool
Ice::SSL::ConnectorI::operator<(const IceInternal::Connector& r) const
{
    const auto* p = dynamic_cast<const ConnectorI*>(&r);
    if (!p)
    {
        return type() < r.type();
    }

    if (this == p)
    {
        return false;
    }

    return Ice::targetLess(_delegate, p->_delegate);
}

Ice::SSL::ConnectorI::ConnectorI(InstancePtr instance, IceInternal::ConnectorPtr del, string h)
    : _instance(std::move(instance)),
      _delegate(std::move(del)),
      _host(std::move(h))
{
}

Ice::SSL::ConnectorI::~ConnectorI() = default;
