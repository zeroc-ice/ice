// Copyright (c) ZeroC, Inc.

#include "NetworkFrameworkTLS.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

#    include "../UniqueRef.h"
#    include "Ice/Connection.h"
#    include "Ice/LocalExceptions.h"
#    include "Ice/LoggerUtil.h"
#    include "Ice/SSL/ConnectionInfo.h"
#    include "ObjectRef.h"

#    include <Block.h>
#    include <Security/Security.h>

#    include <cassert>
#    include <chrono>
#    include <map>
#    include <mutex>
#    include <vector>

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
    SecRef<sec_identity_t> createIdentity(CFArrayRef chain, const char* callbackName)
    {
        UniqueRef<CFArrayRef> holder(chain);
        if (!chain || CFArrayGetCount(chain) == 0)
        {
            return {};
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
            return SecRef<sec_identity_t>::adopt(sec_identity_create(identity));
        }

        UniqueRef<CFMutableArrayRef> intermediates(
            CFArrayCreateMutable(kCFAllocatorDefault, count - 1, &kCFTypeArrayCallBacks));
        for (CFIndex i = 1; i < count; ++i)
        {
            CFArrayAppendValue(intermediates.get(), CFArrayGetValueAtIndex(chain, i));
        }
        return SecRef<sec_identity_t>::adopt(sec_identity_create_with_certificates(identity, intermediates.get()));
    }

    // Selects the server identity of the TLS handshakes of a listener with the server certificate selection
    // callback, calling the callback once per handshake.
    //
    // Network.framework invokes the challenge block twice for the same handshake when the client's ClientHello is
    // already buffered when the handshake starts, which is common with local clients; the second invocation can
    // start before the first one completes, and the handshake uses whichever completion arrives first. Without
    // deduplication the application callback runs twice per connection, and two different answers (for example
    // during a certificate reload) race. The invocations of the same handshake share the same metadata object,
    // which identifies the handshake here; it is retained while cached, so its address cannot be reused by another
    // handshake in the meantime.
    class ServerIdentitySelection
    {
    public:
        explicit ServerIdentitySelection(shared_ptr<const ServerConfiguration> configuration)
            : _configuration(std::move(configuration))
        {
        }

        void select(sec_protocol_metadata_t metadata, sec_protocol_challenge_complete_t complete)
        {
            {
                unique_lock lock(_mutex);
                prune();
                auto p = _handshakes.find(metadata);
                if (p == _handshakes.end())
                {
                    _handshakes.emplace(metadata, Handshake{SecRef<sec_protocol_metadata_t>::retain(metadata)});
                }
                else if (!p->second.selected)
                {
                    // The first invocation is still running the application callback: it completes this one.
                    p->second.waiting.push_back(Block_copy(complete));
                    return;
                }
                else
                {
                    // The second invocation of a handshake whose identity is already selected.
                    SecRef<sec_identity_t> selected = std::move(p->second.identity);
                    _handshakes.erase(p);
                    lock.unlock();
                    complete(selected.get());
                    return;
                }
            }

            SecRef<sec_identity_t> identity = selectIdentity();

            vector<sec_protocol_challenge_complete_t> waiting;
            {
                lock_guard lock(_mutex);
                auto p = _handshakes.find(metadata);
                assert(p != _handshakes.end());
                if (p->second.waiting.empty())
                {
                    // Keep the identity for a second invocation that has not started yet.
                    p->second.selected = true;
                    p->second.identity = SecRef<sec_identity_t>::retain(identity.get());
                    p->second.selectedTime = chrono::steady_clock::now();
                }
                else
                {
                    waiting.swap(p->second.waiting);
                    _handshakes.erase(p);
                }
            }

            complete(identity.get());
            for (auto waitingComplete : waiting)
            {
                waitingComplete(identity.get());
                Block_release(waitingComplete);
            }
        }

    private:
        struct Handshake
        {
            explicit Handshake(SecRef<sec_protocol_metadata_t> handshakeMetadata)
                : metadata(std::move(handshakeMetadata))
            {
            }

            SecRef<sec_protocol_metadata_t> metadata;
            bool selected{false};
            SecRef<sec_identity_t> identity;
            chrono::steady_clock::time_point selectedTime;
            vector<sec_protocol_challenge_complete_t> waiting; // Copied blocks, released once called.
        };

        // The block runs on a dispatch thread, where an escaping exception terminates the process: a failing
        // certificate selection (for example a reload that cannot read the new certificate) fails this handshake
        // instead.
        [[nodiscard]] SecRef<sec_identity_t> selectIdentity() const
        {
            try
            {
                return createIdentity(
                    _configuration->options.serverCertificateSelectionCallback(_configuration->adapterName),
                    "server certificate selection callback");
            }
            catch (const std::exception& ex)
            {
                Warning out(_configuration->logger);
                out << "SSL transport: the server certificate selection callback failed:\n" << ex.what();
            }
            catch (...)
            {
                Warning out(_configuration->logger);
                out << "SSL transport: the server certificate selection callback failed";
            }
            return {};
        }

        // Removes the handshakes that were invoked only once and selected more than the retention ago. The second
        // invocation is issued together with the first one, so the retention only needs to cover scheduling delays;
        // if a second invocation ever came later, the callback would simply run again for that handshake. Pruning
        // runs from select, so an idle listener keeps its last entries until the next handshake or until it is
        // released. Must be called with _mutex locked.
        void prune()
        {
            auto now = chrono::steady_clock::now();
            if (now - _lastPrune < retention)
            {
                return;
            }
            _lastPrune = now;
            for (auto p = _handshakes.begin(); p != _handshakes.end();)
            {
                if (p->second.selected && now - p->second.selectedTime >= retention)
                {
                    p = _handshakes.erase(p);
                }
                else
                {
                    ++p;
                }
            }
        }

        static constexpr chrono::seconds retention{5};

        const shared_ptr<const ServerConfiguration> _configuration;
        mutex _mutex;
        map<sec_protocol_metadata_t, Handshake> _handshakes;
        chrono::steady_clock::time_point _lastPrune;
    };

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
            UniqueRef<CFErrorRef> error;
            return SecTrustEvaluateWithError(trust, &error.get());
        }

        // The callback receives a minimal connection info with the peer certificate so that the trust manager
        // rules (IceSSL.TrustOnly and friends) apply during the handshake, before the connection exists.
        SecCertificateRef peerCertificate = nullptr;
        UniqueRef<CFArrayRef> chain(SecTrustCopyCertificateChain(trust));
        if (chain && CFArrayGetCount(chain.get()) > 0)
        {
            // The leaf comes first. Retained: AppleConnectionInfo releases it.
            peerCertificate =
                static_cast<SecCertificateRef>(const_cast<void*>(CFRetain(CFArrayGetValueAtIndex(chain.get(), 0))));
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
        const shared_ptr<atomic<bool>>& localVerifyRejected,
        sec_protocol_metadata_t metadata,
        sec_protocol_verify_complete_t complete)
    {
        bool valid = false;
        try
        {
            UniqueRef<CFArrayRef> certificates(copyPeerCertificates(metadata));
            UniqueRef<SecPolicyRef> policy(createPolicy(verification));
            UniqueRef<SecTrustRef> trust(
                createPeerTrust(certificates.get(), policy.get(), verification.trustedRootCertificates.get()));
            if (trust)
            {
                valid = validatePeer(trust.get(), verification);
            }
        }
        catch (...)
        {
            valid = false;
        }

        if (!valid && localVerifyRejected)
        {
            localVerifyRejected->store(true);
        }
        complete(valid);
    }

    // Blocks capture a reference parameter as a reference, not a copy: the configuration and the flag are taken by
    // value so that the block captures copies of them, which retain the verification and the objects it uses for as
    // long as the block can run.
    template<typename Configuration>
    void setVerifyBlock(
        sec_protocol_options_t options,
        shared_ptr<const Configuration> configuration, // NOLINT(performance-unnecessary-value-param)
        shared_ptr<atomic<bool>> localVerifyRejected)  // NOLINT(performance-unnecessary-value-param)
    {
        sec_protocol_options_set_verify_block(
            options,
            ^(sec_protocol_metadata_t metadata, sec_trust_t, sec_protocol_verify_complete_t complete) {
              verifyPeer(configuration->verification, localVerifyRejected, metadata, complete);
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
}

IceInternal::NetworkFrameworkTLS::ClientConfiguration::ClientConfiguration(
    SSL::ClientAuthenticationOptions clientOptions,
    string targetHost)
    : options(std::move(clientOptions)),
      host(std::move(targetHost)),
      verification{
          false,
          host,
          UniqueRef<CFArrayRef>(retain(options.trustedRootCertificates)),
          options.serverCertificateValidationCallback}
{
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
          UniqueRef<CFArrayRef>(retain(options.trustedRootCertificates)),
          options.clientCertificateValidationCallback}
{
}

exception_ptr
IceInternal::NetworkFrameworkTLS::configureClientTLS(
    nw_protocol_options_t tlsOptions,
    shared_ptr<const ClientConfiguration> configuration, // NOLINT(performance-unnecessary-value-param)
    shared_ptr<atomic<bool>> localVerifyRejected)        // NOLINT(performance-unnecessary-value-param)
{
    auto options = SecRef<sec_protocol_options_t>::adopt(nw_tls_copy_sec_protocol_options(tlsOptions));
    const SSL::ClientAuthenticationOptions& authOptions = configuration->options;
    const string& host = configuration->host;

    try
    {
        // The server name, for SNI.
        if (!host.empty())
        {
            sec_protocol_options_set_tls_server_name(options.get(), host.c_str());
        }

        // The client identity, selected now: the parameters are created for this connection.
        if (authOptions.clientCertificateSelectionCallback)
        {
            SecRef<sec_identity_t> identity = createIdentity(
                authOptions.clientCertificateSelectionCallback(host),
                "client certificate selection callback");
            if (identity)
            {
                sec_protocol_options_set_local_identity(options.get(), identity.get());
            }
        }

        setVerifyBlock(options.get(), configuration, localVerifyRejected);

        // The application's callback comes last so that the settings it applies take precedence.
        if (authOptions.sslNewSessionCallback)
        {
            authOptions.sslNewSessionCallback(options.get(), host);
        }
    }
    catch (...)
    {
        return current_exception();
    }
    return nullptr;
}

exception_ptr
IceInternal::NetworkFrameworkTLS::configureServerTLS(
    nw_protocol_options_t tlsOptions,
    shared_ptr<const ServerConfiguration> configuration) // NOLINT(performance-unnecessary-value-param)
{
    auto options = SecRef<sec_protocol_options_t>::adopt(nw_tls_copy_sec_protocol_options(tlsOptions));
    const SSL::ServerAuthenticationOptions& authOptions = configuration->options;

    try
    {
        // The TLS options are configured once and shared by all the connections the listener accepts. The server
        // identity is selected from a challenge block, which runs for each TLS handshake: the certificate selection
        // callback is invoked once per connection (see ServerIdentitySelection) and a reloaded certificate is
        // picked up without recreating the listener.
        if (authOptions.serverCertificateSelectionCallback)
        {
            auto selection = make_shared<ServerIdentitySelection>(configuration);
            sec_protocol_options_set_challenge_block(
                options.get(),
                ^(sec_protocol_metadata_t metadata, sec_protocol_challenge_complete_t complete) {
                  selection->select(metadata, complete);
                },
                dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
        }

        // Network.framework only supports binary peer authentication: required or not requested
        // (sec_protocol_options_set_peer_authentication_optional is API_UNAVAILABLE on all platforms), which is why
        // the Apple SSL engine rejects IceSSL.VerifyPeer=1. When a client certificate is required, Network.framework
        // enforces it at the protocol level, before the verify block runs, so clients without a certificate are
        // rejected right away. When it is not required, the server does not request a client certificate; the trust
        // rules still apply to such connections once the handshake completed, see
        // NetworkFrameworkTransceiver::setPeerVerifier.
        if (authOptions.clientCertificateRequired)
        {
            sec_protocol_options_set_peer_authentication_required(options.get(), true);
            // The listener configuration is shared by its connections: there is no per-connection rejection flag.
            setVerifyBlock(options.get(), configuration, nullptr);
        }
        else
        {
            sec_protocol_options_set_peer_authentication_required(options.get(), false);
        }

        // The application's callback comes last so that the settings it applies take precedence.
        if (authOptions.sslNewSessionCallback)
        {
            authOptions.sslNewSessionCallback(options.get(), configuration->adapterName);
        }
    }
    catch (...)
    {
        return current_exception();
    }
    return nullptr;
}

#endif
