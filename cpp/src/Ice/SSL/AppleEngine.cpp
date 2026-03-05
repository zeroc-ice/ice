// Copyright (c) ZeroC, Inc.

#include "AppleEngine.h"
#include "../FileUtil.h"
#include "../Instance.h"
#include "../TraceLevels.h"
#include "Ice/Config.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Logger.h"
#include "Ice/Properties.h"
#include "Ice/SSL/SSLException.h"
#include "Ice/UUID.h"
#include "SSLEngine.h"
#include "SSLUtil.h"
#include "AppleSSLUtil.h"

#include <cerrno>
#include <climits>
#include <cstring>
#include <unistd.h>
#include <vector>

#include "../DisableWarnings.h"

using namespace std;
using namespace Ice;
using namespace Ice::SSL;
using namespace IceInternal;
using namespace Ice::SSL::Apple;

namespace
{
    string trustErrorToString(CFErrorRef err)
    {
        long errorCode = CFErrorGetCode(err);
        switch (errorCode)
        {
            case errSecPathLengthConstraintExceeded:
            {
                return "The path length constraint was exceeded.";
            }
            case errSecUnknownCRLExtension:
            {
                return "An unknown CRL extension was encountered.";
            }
            case errSecUnknownCriticalExtensionFlag:
            {
                return "There is an unknown critical extension flag.";
            }
            case errSecHostNameMismatch:
            {
                return "A host name mismatch has occurred.";
            }
            case errSecNoBasicConstraints:
            {
                return "No basic constraints were found.";
            }
            case errSecNoBasicConstraintsCA:
            {
                return "No basic CA constraints were found.";
            }
            case errSecMissingRequiredExtension:
            {
                return "A required certificate extension is missing.";
            }
            case errSecUnknownCertExtension:
            {
                return "An unknown certificate extension was detected.";
            }
            case errSecCertificateNameNotAllowed:
            {
                return "The requested name isn't allowed for this certificate.";
            }
            case errSecInvalidName:
            {
                return "An invalid name was detected.";
            }
            case errSecInvalidPolicyIdentifiers:
            {
                return "The policy identifiers are not valid.";
            }
            case errSecInvalidCertificateRef:
            {
                return "An invalid certificate reference was detected.";
            }
            case errSecInvalidDigestAlgorithm:
            {
                return "An invalid digest algorithm was detected.";
            }
            case errSecUnsupportedKeySize:
            {
                return "The key size is not supported.";
            }
            case errSecInvalidExtendedKeyUsage:
            {
                return "The extended key usage is not valid.";
            }
            case errSecInvalidKeyUsageForPolicy:
            {
                return "The key usage is not valid for the specified policy.";
            }
            case errSecInvalidSignature:
            {
                return "An invalid signature was detected.";
            }
            case errSecCertificateExpired:
            {
                return "An expired certificate was detected.";
            }
            case errSecCertificateNotValidYet:
            {
                return "The certificate is not yet valid.";
            }
            case errSecCertificateValidityPeriodTooLong:
            {
                return "The validity period in the certificate exceeds the maximum allowed period.";
            }
            case errSecCreateChainFailed:
            {
                return "The attempt to create a certificate chain failed.";
            }
            case errSecCertificateRevoked:
            {
                return "The certificate was revoked.";
            }
            case errSecIncompleteCertRevocationCheck:
            {
                return "An incomplete certificate revocation check occurred.";
            }
            case errSecOCSPNotTrustedToAnchor:
            {
                return "The online certificate status protocol (OCSP) response is not trusted to a root or anchor "
                       "certificate.";
            }
            case errSecNotTrusted:
            {
                return "The trust policy is not trusted.";
            }
            case errSecVerifyActionFailed:
            {
                return "A verify action failed.";
            }
            default:
            {
                ostringstream os;
                os << "An unknown trust failure occurred: " << errorCode;
                return os.str();
            }
        }
    }

#if defined(ICE_USE_APPLE_SSL_MACOS)
    // Creates a private temporary directory to hold a short-lived keychain, and returns its path.
    // Uses confstr(_CS_DARWIN_USER_TEMP_DIR) rather than $TMPDIR so the location can't be
    // redirected through the process environment.
    string createTemporaryKeychainDirectory()
    {
        char base[PATH_MAX];
        size_t len = confstr(_CS_DARWIN_USER_TEMP_DIR, base, sizeof(base));
        if (len == 0 || len > sizeof(base))
        {
            int error = errno;

            ostringstream os;
            os << "SSL transport: confstr(_CS_DARWIN_USER_TEMP_DIR) failed";
            if (error != 0)
            {
                os << ": " << strerror(error);
            }
            throw InitializationException(__FILE__, __LINE__, os.str());
        }

        string tmpl = string{base} + "ice-keychain-XXXXXX";
        vector<char> buffer(tmpl.begin(), tmpl.end());
        buffer.push_back('\0');

        char* dir = mkdtemp(buffer.data());
        if (dir == nullptr)
        {
            int error = errno;
            ostringstream os;
            os << "SSL transport: mkdtemp failed: " << strerror(error);
            throw InitializationException(__FILE__, __LINE__, os.str());
        }
        return dir;
    }
#endif
}

Apple::SSLEngine::SSLEngine(const IceInternal::InstancePtr& instance)
    : Ice::SSL::SSLEngine(instance),
      _certificateAuthorities(nullptr),
      _chain(nullptr)
{
}

Apple::SSLEngine::~SSLEngine()
{
#if defined(ICE_USE_APPLE_SSL_MACOS)
    if (!_temporaryKeychainDir.empty())
    {
        // Remove the temporary keychain and its enclosing directory created by initialize().
        // The cleanup lives in the destructor (not destroy()) so it also runs when initialize()
        // throws after the directory has been created.
        _chain.reset();
        const string keychainPath = _temporaryKeychainDir + "/ice.keychain";
        UniqueRef<SecKeychainRef> keychain;
        if (SecKeychainOpen(keychainPath.c_str(), &keychain.get()) == noErr && keychain.get())
        {
            SecKeychainDelete(keychain.get());
        }
        rmdir(_temporaryKeychainDir.c_str());
    }
#endif
}

//
// Setup the engine.
//
void
Apple::SSLEngine::initialize()
{
    Ice::SSL::SSLEngine::initialize();

    const PropertiesPtr properties = getProperties();

    //
    // Check for a default directory. We look in this directory for
    // files mentioned in the configuration.
    //
    const string defaultDir = properties->getIceProperty("IceSSL.DefaultDir");

    //
    // Load the CA certificates used to authenticate peers into
    // _certificateAuthorities array.
    //
    try
    {
        string caFile = properties->getIceProperty("IceSSL.CAs");
        if (!caFile.empty())
        {
            optional<string> resolved = resolveFilePath(caFile, defaultDir);

            if (!resolved)
            {
                ostringstream os;
                os << "SSL transport: CA certificate file not found: '" << caFile << "'";
                throw InitializationException(__FILE__, __LINE__, os.str());
            }
            _certificateAuthorities.reset(loadCACertificates(*resolved));
        }
        else if (properties->getIcePropertyAsInt("IceSSL.UsePlatformCAs") <= 0)
        {
            // Setup an empty list of Root CAs to not use the system root CAs.
            _certificateAuthorities.reset(CFArrayCreate(nullptr, nullptr, 0, nullptr));
        }
    }
    catch (const CertificateReadException& ce)
    {
        throw InitializationException(__FILE__, __LINE__, ce.what());
    }

    const string password = properties->getIceProperty("IceSSL.Password");

    string certFile = properties->getIceProperty("IceSSL.CertFile");
    string findCert = properties->getIceProperty("IceSSL.FindCert");
    string keychain = properties->getIceProperty("IceSSL.Keychain");
    string keychainPassword = properties->getIceProperty("IceSSL.KeychainPassword");

    if (!certFile.empty())
    {
        optional<string> resolved = resolveFilePath(certFile, defaultDir);

        if (!resolved)
        {
            ostringstream os;
            os << "SSL transport: certificate file not found: '" << certFile << "'";
            throw InitializationException(__FILE__, __LINE__, os.str());
        }
        certFile = *resolved;

        string keyFile = properties->getIceProperty("IceSSL.KeyFile");
        if (!keyFile.empty())
        {
            resolved = resolveFilePath(keyFile, defaultDir);
            if (!resolved)
            {
                ostringstream os;
                os << "SSL transport: key file not found: '" << keyFile << "'";
                throw InitializationException(__FILE__, __LINE__, os.str());
            }
            keyFile = *resolved;
        }

#if defined(ICE_USE_APPLE_SSL_MACOS)
        if (keychain.empty())
        {
            // Import the certificate into a temporary keychain rather than the user's login keychain.
            // A private key in the login keychain cannot complete a forward-secret (ECDHE) handshake
            // on the server side. The keychain and its enclosing directory are removed by the destructor.
            _temporaryKeychainDir = createTemporaryKeychainDirectory();
            keychain = _temporaryKeychainDir + "/ice.keychain";
            keychainPassword = Ice::generateUUID();
        }
#endif

        try
        {
            _chain.reset(loadCertificateChain(certFile, keyFile, keychain, keychainPassword, password));
        }
        catch (const CertificateReadException& ce)
        {
            throw InitializationException(__FILE__, __LINE__, ce.what());
        }
    }
    else if (!findCert.empty())
    {
        _chain.reset(findCertificateChain(keychain, keychainPassword, findCert));
    }
}

ClientAuthenticationOptions
Apple::SSLEngine::createClientAuthenticationOptions(const string& host) const
{
    // It is safe to capture 'this' in the callbacks below as SSLEngine is managed by the communicator
    // and is guaranteed to outlive all connections.
    return ClientAuthenticationOptions{
        .clientCertificateSelectionCallback =
            [this](const string&)
        {
            CFArrayRef chain = _chain.get();
            if (chain)
            {
                CFRetain(chain);
            }
            return chain;
        },
        .sslNewSessionCallback = nullptr,
        .trustedRootCertificates = _certificateAuthorities.get(),
        .serverCertificateValidationCallback = [this, host](SecTrustRef trust, const Ice::SSL::ConnectionInfoPtr& info)
        { return validationCallback(trust, info, host); }};
}

ServerAuthenticationOptions
Apple::SSLEngine::createServerAuthenticationOptions() const
{
    // It is safe to capture 'this' in the callbacks below as SSLEngine is managed by the communicator
    // and is guaranteed to outlive all connections.
    return ServerAuthenticationOptions{
        .serverCertificateSelectionCallback =
            [this](const string&)
        {
            CFArrayRef chain = _chain.get();
            if (chain)
            {
                CFRetain(chain);
            }
            return chain;
        },
        .sslNewSessionCallback = nullptr,
        .clientCertificateRequired = getVerifyPeer() >= 2,
        .trustedRootCertificates = _certificateAuthorities.get(),
        .clientCertificateValidationCallback = [this](SecTrustRef trust, const Ice::SSL::ConnectionInfoPtr& info)
        { return validationCallback(trust, info, ""); }};
}

bool
Apple::SSLEngine::validationCallback(SecTrustRef trust, const ConnectionInfoPtr& info, const string& host)
    const
{
    OSStatus err = noErr;
    UniqueRef<CFErrorRef> trustErr;
    assert(trust);

    // Do not allow to fetch missing intermediate certificates from the network.
    if ((err = SecTrustSetNetworkFetchAllowed(trust, false)))
    {
        throw SecurityException(__FILE__, __LINE__, "SSL transport: handshake failure:\n" + sslErrorToString(err));
    }

    UniqueRef<CFMutableArrayRef> policies(CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks));
    // Add SSL trust policy if we need to check the certificate name, otherwise use basic x509 policy.
    if (getCheckCertName() && !host.empty())
    {
        UniqueRef<CFStringRef> hostref(toCFString(string(host)));
        UniqueRef<SecPolicyRef> policy(SecPolicyCreateSSL(true, hostref.get()));
        CFArrayAppendValue(policies.get(), policy.get());
    }
    else
    {
        UniqueRef<SecPolicyRef> policy(SecPolicyCreateBasicX509());
        CFArrayAppendValue(policies.get(), policy.get());
    }

    int revocationCheck = getRevocationCheck();
    if (revocationCheck > 0)
    {
        CFOptionFlags revocationFlags = kSecRevocationUseAnyAvailableMethod | kSecRevocationRequirePositiveResponse;
        if (getRevocationCheckCacheOnly())
        {
            revocationFlags |= kSecRevocationNetworkAccessDisabled;
        }

        UniqueRef<SecPolicyRef> revocationPolicy(SecPolicyCreateRevocation(revocationFlags));
        if (!revocationPolicy)
        {
            throw SecurityException(
                __FILE__,
                __LINE__,
                "SSL transport: handshake failure: error creating revocation policy");
        }
        CFArrayAppendValue(policies.get(), revocationPolicy.get());
    }

    if ((err = SecTrustSetPolicies(trust, policies.get())))
    {
        throw SecurityException(__FILE__, __LINE__, "SSL transport: handshake failure:\n" + sslErrorToString(err));
    }

    //
    // Evaluate the trust
    //
    if (!SecTrustEvaluateWithError(trust, &trustErr.get()))
    {
        ostringstream os;
        os << "SSL transport: certificate verification failure:\n" << trustErrorToString(trustErr.get());
        string msg = os.str();
        if (instance()->traceLevels()->network >= 1)
        {
            getLogger()->trace(instance()->traceLevels()->networkCat, msg);
        }
        throw SecurityException(__FILE__, __LINE__, msg);
    }
    // Skip trust manager verification when connection info is not yet available (e.g., during the
    // Network.framework TLS handshake where the verify block fires before the connection is established).
    if (info)
    {
        verifyPeer(info);
    }
    return true;
}
