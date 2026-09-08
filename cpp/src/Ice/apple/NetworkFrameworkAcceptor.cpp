// Copyright (c) ZeroC, Inc.

#include "../Network.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

#    include "Ice/Connection.h"
#    include "Ice/LocalExceptions.h"
#    include "Ice/LoggerUtil.h"
#    include "Ice/Properties.h"
#    include "Ice/SSL/ConnectionInfo.h"
#    include "../ProtocolInstance.h"
#    include "../TcpEndpointI.h"
#    include "NetworkFrameworkAcceptor.h"
#    include "NetworkFrameworkTransceiver.h"

#    include <Security/Security.h>

#    include <utility>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    // Creates the sec_identity_t for a certificate chain returned by a certificate selection callback: the
    // identity is the first element, followed by the intermediate certificates Network.framework sends during
    // the TLS handshake so that clients can verify chains with intermediate CAs. Returns null when the chain is
    // empty. The chain is released.
    sec_identity_t createIdentity(CFArrayRef certs)
    {
        if (!certs)
        {
            return nullptr;
        }

        sec_identity_t secIdentity = nullptr;
        CFIndex count = CFArrayGetCount(certs);
        if (count > 0)
        {
            SecIdentityRef identity = (SecIdentityRef)CFArrayGetValueAtIndex(certs, 0);
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
        }
        CFRelease(certs);
        return secIdentity;
    }
}

NativeInfoPtr
IceInternal::NetworkFrameworkAcceptor::getNativeInfo()
{
    return _nativeInfo;
}

void
IceInternal::NetworkFrameworkAcceptor::close()
{
    if (_listener)
    {
        // Cancel the listener and wait for it to reach the cancelled state. This ensures the
        // listening port is fully released before close() returns, preventing "Address already
        // in use" errors when the same port is reused immediately (e.g., adapter destroy/recreate).
        dispatch_semaphore_t sem = dispatch_semaphore_create(0);
        nw_listener_set_state_changed_handler(
            _listener,
            ^(nw_listener_state_t state, [[maybe_unused]] nw_error_t error) {
                if (state == nw_listener_state_cancelled)
                {
                    dispatch_semaphore_signal(sem);
                }
            });
        nw_listener_cancel(_listener);
        dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
        dispatch_release(sem);
    }

    // Release any queued connections that haven't been accepted.
    lock_guard lock(_acceptState->mutex);
    for (auto& conn : _acceptState->connections)
    {
        nw_release(conn);
    }
    _acceptState->connections.clear();

    // If startAccept() is waiting for a connection, signal the completion so the thread pool
    // can process the close and transition IncomingConnectionFactory to StateFinished.
    if (_acceptState->waiting)
    {
        _acceptState->waiting = false;
        _nativeInfo->completed(SocketOperationRead);
    }
}

EndpointIPtr
IceInternal::NetworkFrameworkAcceptor::listen()
{
    //
    // Synchronization state for waiting until the listener is ready.
    //
    struct ListenState
    {
        std::mutex mutex;
        std::condition_variable cv;
        bool signaled{false};
        int error{0};
    };
    auto listenState = make_shared<ListenState>();

    //
    // Set state changed handler to detect when the listener is ready.
    //
    nw_listener_set_state_changed_handler(_listener, ^(nw_listener_state_t state, nw_error_t error) {
        if (state == nw_listener_state_ready || state == nw_listener_state_failed)
        {
            lock_guard lock(listenState->mutex);
            listenState->signaled = true;
            if (state == nw_listener_state_failed)
            {
                listenState->error = error ? nw_error_get_error_code(error) : EADDRINUSE;
            }
            listenState->cv.notify_one();
        }
    });

    //
    // Set new connection handler. When a client connects, the handler queues
    // the nw_connection and, if the acceptor is waiting for a connection,
    // signals the selector so the thread pool can pick it up.
    //
    auto acceptState = _acceptState;
    NativeInfoPtr nativeInfo = _nativeInfo;
    nw_listener_set_new_connection_handler(_listener, ^(nw_connection_t connection) {
        nw_retain(connection);
        lock_guard lock(acceptState->mutex);
        acceptState->connections.push_back(connection);
        if (acceptState->waiting)
        {
            acceptState->waiting = false;
            nativeInfo->completed(SocketOperationRead);
        }
    });

    //
    // Start the listener and wait for it to become ready.
    //
    nw_listener_set_queue(_listener, _dispatchQueue);
    nw_listener_start(_listener);

    {
        unique_lock lock(listenState->mutex);
        listenState->cv.wait(lock, [&] { return listenState->signaled; });
    }

    if (listenState->error != 0)
    {
        throw SocketException(__FILE__, __LINE__, listenState->error);
    }

    //
    // Clear the state changed handler — we no longer need it for the listen flow.
    //
    nw_listener_set_state_changed_handler(_listener, nullptr);

    //
    // Retrieve the actual port assigned by the OS (important for port 0).
    //
    _port = nw_listener_get_port(_listener);

    _endpoint = _endpoint->endpoint(shared_from_this());
    return _endpoint;
}

void
IceInternal::NetworkFrameworkAcceptor::startAccept()
{
    lock_guard lock(_acceptState->mutex);
    if (!_acceptState->connections.empty())
    {
        //
        // A connection is already queued — signal immediately so the
        // thread pool picks it up without waiting.
        //
        _nativeInfo->completed(SocketOperationRead);
    }
    else
    {
        //
        // No connections yet — the new_connection_handler will signal
        // when one arrives.
        //
        _acceptState->waiting = true;
    }
}

void
IceInternal::NetworkFrameworkAcceptor::finishAccept()
{
    // Nothing to do — the connection is already queued in _acceptState
    // and will be retrieved by accept().
}

TransceiverPtr
IceInternal::NetworkFrameworkAcceptor::accept()
{
    nw_connection_t connection;
    {
        lock_guard lock(_acceptState->mutex);
        if (_acceptState->connections.empty())
        {
            throw SocketException(__FILE__, __LINE__, 0);
        }
        connection = _acceptState->connections.front();
        _acceptState->connections.pop_front();
    }

    try
    {
        auto transceiver = make_shared<NetworkFrameworkTransceiver>(_instance, connection, _secure);
        nw_release(connection); // The transceiver retains its own reference.
        return transceiver;
    }
    catch (...)
    {
        nw_release(connection);
        throw;
    }
}

string
IceInternal::NetworkFrameworkAcceptor::protocol() const
{
    return _instance->protocol();
}

string
IceInternal::NetworkFrameworkAcceptor::toString() const
{
    return _host + ":" + to_string(_port);
}

string
IceInternal::NetworkFrameworkAcceptor::toDetailedString() const
{
    return "local address = " + toString();
}

int
IceInternal::NetworkFrameworkAcceptor::effectivePort() const
{
    return _port;
}

IceInternal::NetworkFrameworkAcceptor::NetworkFrameworkAcceptor(
    TcpEndpointIPtr endpoint,
    const ProtocolInstancePtr& instance,
    const string& host,
    int port,
    const string& adapterName,
    const optional<Ice::SSL::ServerAuthenticationOptions>& serverAuthenticationOptions)
    : _endpoint(std::move(endpoint)),
      _instance(instance),
      _nativeInfo(make_shared<NativeInfo>(INVALID_SOCKET)),
      _listener(nullptr),
      _dispatchQueue(nullptr),
      _host(host),
      _port(static_cast<uint16_t>(port)),
      _secure(serverAuthenticationOptions.has_value())
{
    //
    // Create TCP parameters — with or without TLS.
    //
    nw_parameters_t parameters;
    if (serverAuthenticationOptions)
    {
        auto authOptions = *serverAuthenticationOptions;
        // The blocks below outlive this constructor: capture a copy of the adapter name, not the reference.
        const string name = adapterName;
        parameters = nw_parameters_create_secure_tcp(
            ^(nw_protocol_options_t tlsOptions) {
                sec_protocol_options_t secOptions = nw_tls_copy_sec_protocol_options(tlsOptions);

                // The TLS options are configured once, when the listener is created, and shared by all the
                // connections it accepts. Select the server identity from a challenge block instead of setting
                // it here: the block runs for each TLS handshake, so the certificate selection callback is
                // invoked per connection and the server picks up a new certificate without recreating the
                // listener (hot reload).
                if (authOptions.serverCertificateSelectionCallback)
                {
                    auto certCallback = authOptions.serverCertificateSelectionCallback;
                    dispatch_queue_t challengeQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
                    sec_protocol_options_set_challenge_block(
                        secOptions,
                        ^(sec_protocol_metadata_t, sec_protocol_challenge_complete_t complete) {
                            sec_identity_t secIdentity = createIdentity(certCallback(name));
                            complete(secIdentity);
                            if (secIdentity)
                            {
                                sec_release(secIdentity);
                            }
                        },
                        challengeQueue);
                }

                // Configure client certificate authentication.
                //
                // Network.framework only supports binary peer authentication: required or not
                // requested. The sec_protocol_options_set_peer_authentication_optional() API exists
                // in headers but is API_UNAVAILABLE on all platforms. When peer_authentication_required
                // is true, NF enforces the requirement at the TLS protocol level (before the verify
                // block fires), so clients without certificates are rejected immediately.
                //
                // As a result, IceSSL.VerifyPeer=1 (try authenticate) cannot be faithfully
                // implemented. We treat it as "not required": the server does not request a
                // client certificate. This preserves the primary behavior of allowing clients
                // without certificates to connect, but means client certificates are not verified
                // even when the client has one (since the server never requests it).
                bool clientCertRequired = authOptions.clientCertificateRequired;
                if (clientCertRequired)
                {
                    CFArrayRef trustedRoots = authOptions.trustedRootCertificates;
                    auto validationCallback = authOptions.clientCertificateValidationCallback;

                    sec_protocol_options_set_peer_authentication_required(secOptions, true);

                    dispatch_queue_t verifyQueue =
                        dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
                    sec_protocol_options_set_verify_block(
                        secOptions,
                        ^(sec_protocol_metadata_t metadata, sec_trust_t trustRef, sec_protocol_verify_complete_t complete) {
                            // NF's sec_trust_copy_ref only provides the leaf certificate. Extract
                            // the full peer certificate chain from the protocol metadata.
                            CFMutableArrayRef peerCerts =
                                CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
                            sec_protocol_metadata_access_peer_certificate_chain(
                                metadata,
                                ^(sec_certificate_t cert) {
                                    SecCertificateRef secCert = sec_certificate_copy_ref(cert);
                                    CFArrayAppendValue(peerCerts, secCert);
                                    CFRelease(secCert);
                                });

                            SecPolicyRef policy = SecPolicyCreateBasicX509();
                            SecTrustRef trust;
                            OSStatus status = SecTrustCreateWithCertificates(peerCerts, policy, &trust);
                            CFRelease(policy);
                            CFRelease(peerCerts);

                            if (status != errSecSuccess || !trust)
                            {
                                complete(false);
                                return;
                            }

                            if (trustedRoots)
                            {
                                SecTrustSetAnchorCertificates(trust, trustedRoots);
                                SecTrustSetAnchorCertificatesOnly(trust, true);
                            }

                            if (validationCallback)
                            {
                                try
                                {
                                    // Construct a minimal ConnectionInfo with the peer certificate
                                    // for TrustOnly and other DN-based checks. Server-side: incoming=true.
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
                                        true, name, "", "", 0, "", 0, 0, 0);
                                    auto info = make_shared<Ice::SSL::AppleConnectionInfo>(
                                        underlying, peerCert);
                                    bool valid = validationCallback(trust, info);
                                    complete(valid);
                                }
                                catch (...)
                                {
                                    complete(false);
                                }
                            }
                            else
                            {
                                CFErrorRef error = nullptr;
                                bool valid = SecTrustEvaluateWithError(trust, &error);
                                if (error)
                                {
                                    CFRelease(error);
                                }
                                complete(valid);
                            }
                            CFRelease(trust);
                        },
                        verifyQueue);
                }
                else
                {
                    // Client certificate not required — do not request one.
                    sec_protocol_options_set_peer_authentication_required(secOptions, false);
                }

                // Invoke the application's callback last so that the settings it applies take precedence over the
                // configuration above.
                if (authOptions.sslNewSessionCallback)
                {
                    authOptions.sslNewSessionCallback(secOptions, name);
                }

                nw_release(secOptions); // nw_tls_copy_sec_protocol_options returns a retained object.
            },
            NW_PARAMETERS_DEFAULT_CONFIGURATION);
    }
    else
    {
        parameters = nw_parameters_create_secure_tcp(
            NW_PARAMETERS_DISABLE_PROTOCOL,
            NW_PARAMETERS_DEFAULT_CONFIGURATION);
    }

    if (!parameters)
    {
        throw SocketException(__FILE__, __LINE__, 0);
    }

    //
    // If a specific local address is requested, set it on the parameters.
    // Use port "0" here — the actual listen port is specified separately
    // via nw_listener_create_with_port to avoid specifying the port twice.
    //
    if (!host.empty() && host != "0.0.0.0" && host != "::" && host != "*")
    {
        nw_endpoint_t localEndpoint = nw_endpoint_create_host(host.c_str(), "0");
        if (localEndpoint)
        {
            nw_parameters_set_local_endpoint(parameters, localEndpoint);
            nw_release(localEndpoint);
        }
    }

    //
    // Create the listener — with a specific port or an OS-assigned port.
    //
    if (port > 0)
    {
        _listener = nw_listener_create_with_port(to_string(port).c_str(), parameters);
    }
    else
    {
        _listener = nw_listener_create(parameters);
    }
    nw_release(parameters);

    if (!_listener)
    {
        throw SocketException(__FILE__, __LINE__, 0);
    }

    _dispatchQueue = dispatch_queue_create("com.zeroc.ice.nw-listener", DISPATCH_QUEUE_SERIAL);
    _acceptState = make_shared<AcceptState>();
}

IceInternal::NetworkFrameworkAcceptor::~NetworkFrameworkAcceptor()
{
    if (_listener)
    {
        nw_release(_listener);
    }
    if (_dispatchQueue)
    {
        dispatch_release(_dispatchQueue);
    }
}

#endif
