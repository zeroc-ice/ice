// Copyright (c) ZeroC, Inc.

#include "NetworkFrameworkTLS.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

#    include "../UniqueRef.h"
#    include "Ice/Connection.h"
#    include "Ice/LocalExceptions.h"
#    include "Ice/LoggerUtil.h"
#    include "Ice/SSL/ConnectionInfo.h"

#    include <Security/Security.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceInternal::NetworkFrameworkTLS;

namespace
{
    // Creates the sec_identity_t for the certificate chain a certificate selection callback returns: the identity
    // first, followed by the intermediate certificates Network.framework sends during the handshake so that peers
    // can verify chains with intermediate CAs. Returns null for an empty chain and throws when the first element is
    // not an identity: passing a certificate as the identity crashes Network.framework when it looks up the private
    // key. The chain is released. This conversion disappears with the identity selection callbacks of #6720.
    sec_identity_t createIdentity(CFArrayRef chain, const char* callbackName)
    {
        UniqueRef<CFArrayRef> holder(chain);
        if (!chain || CFArrayGetCount(chain) == 0)
        {
            return nullptr;
        }

        if (CFGetTypeID(CFArrayGetValueAtIndex(chain, 0)) != SecIdentityGetTypeID())
        {
            throw SecurityException(
                __FILE__,
                __LINE__,
                string{"SSL transport: the "} + callbackName +
                    " returned a certificate chain whose first element is not an identity (SecIdentityRef)");
        }

        auto identity = static_cast<SecIdentityRef>(const_cast<void*>(CFArrayGetValueAtIndex(chain, 0)));
        CFIndex count = CFArrayGetCount(chain);
        if (count == 1)
        {
            return sec_identity_create(identity);
        }

        UniqueRef<CFMutableArrayRef> intermediates(
            CFArrayCreateMutable(kCFAllocatorDefault, count - 1, &kCFTypeArrayCallBacks));
        for (CFIndex i = 1; i < count; ++i)
        {
            CFArrayAppendValue(intermediates.get(), CFArrayGetValueAtIndex(chain, i));
        }
        return sec_identity_create_with_certificates(identity, intermediates.get());
    }

    // The certificate chain the peer presented, from the handshake metadata: sec_trust_copy_ref only provides the
    // leaf certificate, the intermediates are needed to evaluate the trust.
    CFArrayRef copyPeerCertificates(sec_protocol_metadata_t metadata)
    {
        CFMutableArrayRef certificates = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
        sec_protocol_metadata_access_peer_certificate_chain(metadata, ^(sec_certificate_t certificate) {
          UniqueRef<SecCertificateRef> ref(sec_certificate_copy_ref(certificate));
          CFArrayAppendValue(certificates, ref.get());
        });
        return certificates;
    }

    // The policy the peer certificate chain is evaluated against: the SSL policy with the host name for an outgoing
    // connection to a named host, the basic X.509 policy otherwise.
    SecPolicyRef createPolicy(const PeerVerification& verification)
    {
        if (!verification.incoming && !verification.name.empty())
        {
            UniqueRef<CFStringRef> host(
                CFStringCreateWithCString(kCFAllocatorDefault, verification.name.c_str(), kCFStringEncodingUTF8));
            return SecPolicyCreateSSL(true, host.get());
        }
        return SecPolicyCreateBasicX509();
    }

    // Creates the trust evaluating the peer certificates against the policy, anchored to the trusted root
    // certificates when set (only those roots are trusted then). Returns null when the trust cannot be created.
    SecTrustRef createPeerTrust(CFArrayRef certificates, SecPolicyRef policy, CFArrayRef trustedRootCertificates)
    {
        SecTrustRef trust = nullptr;
        if (SecTrustCreateWithCertificates(certificates, policy, &trust) != errSecSuccess || !trust)
        {
            return nullptr;
        }
        if (trustedRootCertificates)
        {
            SecTrustSetAnchorCertificates(trust, trustedRootCertificates);
            SecTrustSetAnchorCertificatesOnly(trust, true);
        }
        return trust;
    }

    // Validates the peer with the application's callback, or with the default trust evaluation.
    bool validatePeer(SecTrustRef trust, const PeerVerification& verification)
    {
        if (!verification.callback)
        {
            CFErrorRef error = nullptr;
            bool valid = SecTrustEvaluateWithError(trust, &error);
            if (error)
            {
                CFRelease(error);
            }
            return valid;
        }

        // The callback receives a minimal connection info with the peer certificate so that the trust manager
        // rules (IceSSL.TrustOnly and friends) apply during the handshake, before the connection exists.
        SecCertificateRef peerCertificate = nullptr;
        if (SecTrustGetCertificateCount(trust) > 0)
        {
            peerCertificate = SecTrustGetCertificateAtIndex(trust, 0); // Unretained, valid while the trust is alive.
            if (peerCertificate)
            {
                CFRetain(peerCertificate); // AppleConnectionInfo releases it.
            }
        }
        auto underlying = make_shared<TCPConnectionInfo>(
            verification.incoming,
            verification.incoming ? verification.name : "",
            "",
            "",
            0,
            "",
            0,
            0,
            0);
        return verification.callback(trust, make_shared<SSL::AppleConnectionInfo>(underlying, peerCertificate));
    }

    // The TLS verify block: invokes the completion exactly once, an application exception or a failure to build the
    // trust rejects the peer, and a rejection is recorded in localVerifyRejected when set.
    void verifyPeer(
        const PeerVerification& verification,
        sec_protocol_metadata_t metadata,
        sec_protocol_verify_complete_t complete)
    {
        bool valid = false;
        try
        {
            UniqueRef<CFArrayRef> certificates(copyPeerCertificates(metadata));
            UniqueRef<SecPolicyRef> policy(createPolicy(verification));
            UniqueRef<SecTrustRef> trust(
                createPeerTrust(certificates.get(), policy.get(), verification.trustedRootCertificates));
            if (trust)
            {
                valid = validatePeer(trust.get(), verification);
            }
        }
        catch (...)
        {
            valid = false;
        }

        if (!valid && verification.localVerifyRejected)
        {
            verification.localVerifyRejected->store(true);
        }
        complete(valid);
    }

    // Blocks capture a reference parameter as a reference, not a copy: the blocks below capture a local copy of
    // the configuration, which retains the verification and the objects it uses for as long as they run.
    template<typename Configuration>
    void setVerifyBlock(sec_protocol_options_t options, const shared_ptr<Configuration>& configuration)
    {
        shared_ptr<Configuration> retained = configuration;
        sec_protocol_options_set_verify_block(
            options,
            ^(sec_protocol_metadata_t metadata, sec_trust_t, sec_protocol_verify_complete_t complete) {
              verifyPeer(retained->verification, metadata, complete);
            },
            dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
    }

    CFArrayRef retain(CFArrayRef array)
    {
        if (array)
        {
            CFRetain(array);
        }
        return array;
    }

    void release(CFArrayRef array)
    {
        if (array)
        {
            CFRelease(array);
        }
    }
}

IceInternal::NetworkFrameworkTLS::ClientConfiguration::ClientConfiguration(
    SSL::ClientAuthenticationOptions clientOptions,
    string targetHost)
    : options(std::move(clientOptions)),
      host(std::move(targetHost)),
      verification{
          false,
          host,
          retain(options.trustedRootCertificates),
          options.serverCertificateValidationCallback,
          make_shared<atomic<bool>>(false)}
{
}

IceInternal::NetworkFrameworkTLS::ClientConfiguration::~ClientConfiguration()
{
    release(verification.trustedRootCertificates);
}

IceInternal::NetworkFrameworkTLS::ServerConfiguration::ServerConfiguration(
    SSL::ServerAuthenticationOptions serverOptions,
    string name,
    LoggerPtr serverLogger)
    : options(std::move(serverOptions)),
      adapterName(std::move(name)),
      logger(std::move(serverLogger)),
      verification{
          true,
          adapterName,
          retain(options.trustedRootCertificates),
          options.clientCertificateValidationCallback,
          nullptr}
{
}

IceInternal::NetworkFrameworkTLS::ServerConfiguration::~ServerConfiguration()
{
    release(verification.trustedRootCertificates);
}

void
IceInternal::NetworkFrameworkTLS::configureClientTLS(
    nw_protocol_options_t tlsOptions,
    const shared_ptr<ClientConfiguration>& configuration)
{
    sec_protocol_options_t options = nw_tls_copy_sec_protocol_options(tlsOptions);
    const SSL::ClientAuthenticationOptions& authOptions = configuration->options;
    const string& host = configuration->host;

    try
    {
        // The server name, for SNI.
        if (!host.empty())
        {
            sec_protocol_options_set_tls_server_name(options, host.c_str());
        }

        // The client identity, selected now: the parameters are created for this connection.
        if (authOptions.clientCertificateSelectionCallback)
        {
            sec_identity_t identity = createIdentity(
                authOptions.clientCertificateSelectionCallback(host),
                "client certificate selection callback");
            if (identity)
            {
                sec_protocol_options_set_local_identity(options, identity);
                sec_release(identity);
            }
        }

        setVerifyBlock(options, configuration);

        // The application's callback comes last so that the settings it applies take precedence.
        if (authOptions.sslNewSessionCallback)
        {
            authOptions.sslNewSessionCallback(options, host);
        }
    }
    catch (...)
    {
        configuration->error = current_exception();
    }

    nw_release(options); // nw_tls_copy_sec_protocol_options returns a retained object.
}

void
IceInternal::NetworkFrameworkTLS::configureServerTLS(
    nw_protocol_options_t tlsOptions,
    const shared_ptr<ServerConfiguration>& configuration)
{
    sec_protocol_options_t options = nw_tls_copy_sec_protocol_options(tlsOptions);
    const SSL::ServerAuthenticationOptions& authOptions = configuration->options;

    try
    {
        // The TLS options are configured once and shared by all the connections the listener accepts. The server
        // identity is selected from a challenge block, which runs for each TLS handshake: the certificate selection
        // callback is invoked per connection and a reloaded certificate is picked up without recreating the
        // listener.
        if (authOptions.serverCertificateSelectionCallback)
        {
            shared_ptr<ServerConfiguration> retained = configuration; // See setVerifyBlock.
            sec_protocol_options_set_challenge_block(
                options,
                ^(sec_protocol_metadata_t, sec_protocol_challenge_complete_t complete) {
                  // The block runs on a dispatch thread, where an escaping exception terminates the process: a
                  // failing certificate selection (for example a reload that cannot read the new certificate)
                  // fails this handshake instead.
                  sec_identity_t identity = nullptr;
                  try
                  {
                      identity = createIdentity(
                          retained->options.serverCertificateSelectionCallback(retained->adapterName),
                          "server certificate selection callback");
                  }
                  catch (const std::exception& ex)
                  {
                      Warning out(retained->logger);
                      out << "SSL transport: the server certificate selection callback failed:\n" << ex.what();
                  }
                  catch (...)
                  {
                      Warning out(retained->logger);
                      out << "SSL transport: the server certificate selection callback failed";
                  }
                  complete(identity);
                  if (identity)
                  {
                      sec_release(identity);
                  }
                },
                dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
        }

        // Network.framework only supports binary peer authentication: required or not requested
        // (sec_protocol_options_set_peer_authentication_optional is API_UNAVAILABLE on all platforms). When a
        // client certificate is required, Network.framework enforces it at the protocol level, before the verify
        // block runs, so clients without a certificate are rejected right away. IceSSL.VerifyPeer=1 (try to
        // authenticate) therefore cannot be implemented faithfully: it is treated as "not required", the server
        // does not request a client certificate. Clients without a certificate can connect, which is the primary
        // behavior, but a certificate a client has is not verified. The trust rules still apply to such connections
        // once the handshake completed, see NetworkFrameworkTransceiver::setPeerVerifier.
        if (authOptions.clientCertificateRequired)
        {
            sec_protocol_options_set_peer_authentication_required(options, true);
            setVerifyBlock(options, configuration);
        }
        else
        {
            sec_protocol_options_set_peer_authentication_required(options, false);
        }

        // The application's callback comes last so that the settings it applies take precedence.
        if (authOptions.sslNewSessionCallback)
        {
            authOptions.sslNewSessionCallback(options, configuration->adapterName);
        }
    }
    catch (...)
    {
        configuration->error = current_exception();
    }

    nw_release(options); // nw_tls_copy_sec_protocol_options returns a retained object.
}

#endif
