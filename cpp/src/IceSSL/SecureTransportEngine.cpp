//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SecureTransportEngine.h"
#include "Ice/Certificate.h"
#include "Ice/Config.h"
#include "Ice/LocalException.h"
#include "Ice/Logger.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "IceUtil/FileUtil.h"
#include "IceUtil/StringUtil.h"
#include "SSLEngine.h"
#include "SSLUtil.h"
#include "SecureTransportEngineF.h"
#include "SecureTransportTransceiverI.h"
#include "SecureTransportUtil.h"

// Disable deprecation warnings from SecureTransport APIs
#include "IceUtil/DisableWarnings.h"

using namespace std;
using namespace IceUtil;
using namespace Ice;
using namespace IceInternal;
using namespace IceSSL;
using namespace IceSSL::SecureTransport;

namespace
{
    //
    // Retrieve the name of a cipher, SSLCipherSuite includes duplicated values for TLS/SSL
    // protocol ciphers, for example SSL_RSA_WITH_RC4_128_MD5/TLS_RSA_WITH_RC4_128_MD5
    // are represented by the same SSLCipherSuite value, the names return by this method
    // doesn't include a protocol prefix.
    //
    string cipherName(SSLCipherSuite cipher)
    {
        switch (cipher)
        {
            case SSL_NULL_WITH_NULL_NULL:
                return "NULL_WITH_NULL_NULL";
            case SSL_RSA_WITH_NULL_MD5:
                return "RSA_WITH_NULL_MD5";
            case SSL_RSA_WITH_NULL_SHA:
                return "RSA_WITH_NULL_SHA";
            case SSL_RSA_EXPORT_WITH_RC4_40_MD5:
                return "RSA_EXPORT_WITH_RC4_40_MD5";
            case SSL_RSA_WITH_RC4_128_MD5:
                return "RSA_WITH_RC4_128_MD5";
            case SSL_RSA_WITH_RC4_128_SHA:
                return "RSA_WITH_RC4_128_SHA";
            case SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5:
                return "RSA_EXPORT_WITH_RC2_CBC_40_MD5";
            case SSL_RSA_WITH_IDEA_CBC_SHA:
                return "RSA_WITH_IDEA_CBC_SHA";
            case SSL_RSA_EXPORT_WITH_DES40_CBC_SHA:
                return "RSA_EXPORT_WITH_DES40_CBC_SHA";
            case SSL_RSA_WITH_DES_CBC_SHA:
                return "RSA_WITH_DES_CBC_SHA";
            case SSL_RSA_WITH_3DES_EDE_CBC_SHA:
                return "RSA_WITH_3DES_EDE_CBC_SHA";
            case SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA:
                return "DH_DSS_EXPORT_WITH_DES40_CBC_SHA";
            case SSL_DH_DSS_WITH_DES_CBC_SHA:
                return "DH_DSS_WITH_DES_CBC_SHA";
            case SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA:
                return "DH_DSS_WITH_3DES_EDE_CBC_SHA";
            case SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA:
                return "DH_RSA_EXPORT_WITH_DES40_CBC_SHA";
            case SSL_DH_RSA_WITH_DES_CBC_SHA:
                return "DH_RSA_WITH_DES_CBC_SHA";
            case SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA:
                return "DH_RSA_WITH_3DES_EDE_CBC_SHA";
            case SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA:
                return "DHE_DSS_EXPORT_WITH_DES40_CBC_SHA";
            case SSL_DHE_DSS_WITH_DES_CBC_SHA:
                return "DHE_DSS_WITH_DES_CBC_SHA";
            case SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA:
                return "DHE_DSS_WITH_3DES_EDE_CBC_SHA";
            case SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA:
                return "DHE_RSA_EXPORT_WITH_DES40_CBC_SHA";
            case SSL_DHE_RSA_WITH_DES_CBC_SHA:
                return "DHE_RSA_WITH_DES_CBC_SHA";
            case SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA:
                return "DHE_RSA_WITH_3DES_EDE_CBC_SHA";
            case SSL_DH_anon_EXPORT_WITH_RC4_40_MD5:
                return "DH_anon_EXPORT_WITH_RC4_40_MD5";
            case SSL_DH_anon_WITH_RC4_128_MD5:
                return "DH_anon_WITH_RC4_128_MD5";
            case SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA:
                return "DH_anon_EXPORT_WITH_DES40_CBC_SHA";
            case SSL_DH_anon_WITH_DES_CBC_SHA:
                return "DH_anon_WITH_DES_CBC_SHA";
            case SSL_DH_anon_WITH_3DES_EDE_CBC_SHA:
                return "DH_anon_WITH_3DES_EDE_CBC_SHA";
            case SSL_FORTEZZA_DMS_WITH_NULL_SHA:
                return "FORTEZZA_DMS_WITH_NULL_SHA";
            case SSL_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA:
                return "FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA";

            //
            // TLS addenda using AES, per RFC 3268
            //
            case TLS_RSA_WITH_AES_128_CBC_SHA:
                return "RSA_WITH_AES_128_CBC_SHA";
            case TLS_DH_DSS_WITH_AES_128_CBC_SHA:
                return "DH_DSS_WITH_AES_128_CBC_SHA";
            case TLS_DH_RSA_WITH_AES_128_CBC_SHA:
                return "DH_RSA_WITH_AES_128_CBC_SHA";
            case TLS_DHE_DSS_WITH_AES_128_CBC_SHA:
                return "DHE_DSS_WITH_AES_128_CBC_SHA";
            case TLS_DHE_RSA_WITH_AES_128_CBC_SHA:
                return "DHE_RSA_WITH_AES_128_CBC_SHA";
            case TLS_DH_anon_WITH_AES_128_CBC_SHA:
                return "DH_anon_WITH_AES_128_CBC_SHA";
            case TLS_RSA_WITH_AES_256_CBC_SHA:
                return "RSA_WITH_AES_256_CBC_SHA";
            case TLS_DH_DSS_WITH_AES_256_CBC_SHA:
                return "DH_DSS_WITH_AES_256_CBC_SHA";
            case TLS_DH_RSA_WITH_AES_256_CBC_SHA:
                return "DH_RSA_WITH_AES_256_CBC_SHA";
            case TLS_DHE_DSS_WITH_AES_256_CBC_SHA:
                return "DHE_DSS_WITH_AES_256_CBC_SHA";
            case TLS_DHE_RSA_WITH_AES_256_CBC_SHA:
                return "DHE_RSA_WITH_AES_256_CBC_SHA";
            case TLS_DH_anon_WITH_AES_256_CBC_SHA:
                return "DH_anon_WITH_AES_256_CBC_SHA";

            //
            // ECDSA addenda, RFC 4492
            //
            case TLS_ECDH_ECDSA_WITH_NULL_SHA:
                return "ECDH_ECDSA_WITH_NULL_SHA";
            case TLS_ECDH_ECDSA_WITH_RC4_128_SHA:
                return "ECDH_ECDSA_WITH_RC4_128_SHA";
            case TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA:
                return "ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA";
            case TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA:
                return "ECDH_ECDSA_WITH_AES_128_CBC_SHA";
            case TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA:
                return "ECDH_ECDSA_WITH_AES_256_CBC_SHA";
            case TLS_ECDHE_ECDSA_WITH_NULL_SHA:
                return "ECDHE_ECDSA_WITH_NULL_SHA";
            case TLS_ECDHE_ECDSA_WITH_RC4_128_SHA:
                return "ECDHE_ECDSA_WITH_RC4_128_SHA";
            case TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA:
                return "ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA";
            case TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA:
                return "ECDHE_ECDSA_WITH_AES_128_CBC_SHA";
            case TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA:
                return "ECDHE_ECDSA_WITH_AES_256_CBC_SHA";
            case TLS_ECDH_RSA_WITH_NULL_SHA:
                return "ECDH_RSA_WITH_NULL_SHA";
            case TLS_ECDH_RSA_WITH_RC4_128_SHA:
                return "ECDH_RSA_WITH_RC4_128_SHA";
            case TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA:
                return "ECDH_RSA_WITH_3DES_EDE_CBC_SHA";
            case TLS_ECDH_RSA_WITH_AES_128_CBC_SHA:
                return "ECDH_RSA_WITH_AES_128_CBC_SHA";
            case TLS_ECDH_RSA_WITH_AES_256_CBC_SHA:
                return "ECDH_RSA_WITH_AES_256_CBC_SHA";
            case TLS_ECDHE_RSA_WITH_NULL_SHA:
                return "ECDHE_RSA_WITH_NULL_SHA";
            case TLS_ECDHE_RSA_WITH_RC4_128_SHA:
                return "ECDHE_RSA_WITH_RC4_128_SHA";
            case TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA:
                return "ECDHE_RSA_WITH_3DES_EDE_CBC_SHA";
            case TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA:
                return "ECDHE_RSA_WITH_AES_128_CBC_SHA";
            case TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA:
                return "ECDHE_RSA_WITH_AES_256_CBC_SHA";
            case TLS_ECDH_anon_WITH_NULL_SHA:
                return "ECDH_anon_WITH_NULL_SHA";
            case TLS_ECDH_anon_WITH_RC4_128_SHA:
                return "ECDH_anon_WITH_RC4_128_SHA";
            case TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA:
                return "ECDH_anon_WITH_3DES_EDE_CBC_SHA";
            case TLS_ECDH_anon_WITH_AES_128_CBC_SHA:
                return "ECDH_anon_WITH_AES_128_CBC_SHA";
            case TLS_ECDH_anon_WITH_AES_256_CBC_SHA:
                return "ECDH_anon_WITH_AES_256_CBC_SHA";

            //
            // TLS 1.2 addenda, RFC 5246
            //
            // case TLS_NULL_WITH_NULL_NULL:
            //    return "NULL_WITH_NULL_NULL";

            //
            // Server provided RSA certificate for key exchange.
            //
            // case TLS_RSA_WITH_NULL_MD5:
            //    return "RSA_WITH_NULL_MD5";
            // case TLS_RSA_WITH_NULL_SHA:
            //    return "RSA_WITH_NULL_SHA";
            // case TLS_RSA_WITH_RC4_128_MD5:
            //    return "RSA_WITH_RC4_128_MD5";
            // case TLS_RSA_WITH_RC4_128_SHA:
            //    return "RSA_WITH_RC4_128_SHA";
            // case TLS_RSA_WITH_3DES_EDE_CBC_SHA:
            //    return "RSA_WITH_3DES_EDE_CBC_SHA";
            case TLS_RSA_WITH_NULL_SHA256:
                return "RSA_WITH_NULL_SHA256";
            case TLS_RSA_WITH_AES_128_CBC_SHA256:
                return "RSA_WITH_AES_128_CBC_SHA256";
            case TLS_RSA_WITH_AES_256_CBC_SHA256:
                return "RSA_WITH_AES_256_CBC_SHA256";

            //
            // Server-authenticated (and optionally client-authenticated) Diffie-Hellman.
            //
            // case TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA:
            //    return "DH_DSS_WITH_3DES_EDE_CBC_SHA";
            // case TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA:
            //    return "DH_RSA_WITH_3DES_EDE_CBC_SHA";
            // case TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA:
            //    return "DHE_DSS_WITH_3DES_EDE_CBC_SHA";
            // case TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA:
            //    return "DHE_RSA_WITH_3DES_EDE_CBC_SHA";
            case TLS_DH_DSS_WITH_AES_128_CBC_SHA256:
                return "DH_DSS_WITH_AES_128_CBC_SHA256";
            case TLS_DH_RSA_WITH_AES_128_CBC_SHA256:
                return "DH_RSA_WITH_AES_128_CBC_SHA256";
            case TLS_DHE_DSS_WITH_AES_128_CBC_SHA256:
                return "DHE_DSS_WITH_AES_128_CBC_SHA256";
            case TLS_DHE_RSA_WITH_AES_128_CBC_SHA256:
                return "DHE_RSA_WITH_AES_128_CBC_SHA256";
            case TLS_DH_DSS_WITH_AES_256_CBC_SHA256:
                return "DH_DSS_WITH_AES_256_CBC_SHA256";
            case TLS_DH_RSA_WITH_AES_256_CBC_SHA256:
                return "DH_RSA_WITH_AES_256_CBC_SHA256";
            case TLS_DHE_DSS_WITH_AES_256_CBC_SHA256:
                return "DHE_DSS_WITH_AES_256_CBC_SHA256";
            case TLS_DHE_RSA_WITH_AES_256_CBC_SHA256:
                return "DHE_RSA_WITH_AES_256_CBC_SHA256";

            //
            // Completely anonymous Diffie-Hellman
            //
            // case TLS_DH_anon_WITH_RC4_128_MD5:
            //    return "DH_anon_WITH_RC4_128_MD5";
            // case TLS_DH_anon_WITH_3DES_EDE_CBC_SHA:
            //    return "DH_anon_WITH_3DES_EDE_CBC_SHA";
            case TLS_DH_anon_WITH_AES_128_CBC_SHA256:
                return "DH_anon_WITH_AES_128_CBC_SHA256";
            case TLS_DH_anon_WITH_AES_256_CBC_SHA256:
                return "DH_anon_WITH_AES_256_CBC_SHA256";

            //
            // Addendum from RFC 4279, TLS PSK
            //
            case TLS_PSK_WITH_RC4_128_SHA:
                return "PSK_WITH_RC4_128_SHA";
            case TLS_PSK_WITH_3DES_EDE_CBC_SHA:
                return "PSK_WITH_3DES_EDE_CBC_SHA";
            case TLS_PSK_WITH_AES_128_CBC_SHA:
                return "PSK_WITH_AES_128_CBC_SHA";
            case TLS_PSK_WITH_AES_256_CBC_SHA:
                return "PSK_WITH_AES_256_CBC_SHA";
            case TLS_DHE_PSK_WITH_RC4_128_SHA:
                return "DHE_PSK_WITH_RC4_128_SHA";
            case TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA:
                return "DHE_PSK_WITH_3DES_EDE_CBC_SHA";
            case TLS_DHE_PSK_WITH_AES_128_CBC_SHA:
                return "DHE_PSK_WITH_AES_128_CBC_SHA";
            case TLS_DHE_PSK_WITH_AES_256_CBC_SHA:
                return "DHE_PSK_WITH_AES_256_CBC_SHA";
            case TLS_RSA_PSK_WITH_RC4_128_SHA:
                return "RSA_PSK_WITH_RC4_128_SHA";
            case TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA:
                return "RSA_PSK_WITH_3DES_EDE_CBC_SHA";
            case TLS_RSA_PSK_WITH_AES_128_CBC_SHA:
                return "RSA_PSK_WITH_AES_128_CBC_SHA";
            case TLS_RSA_PSK_WITH_AES_256_CBC_SHA:
                return "RSA_PSK_WITH_AES_256_CBC_SHA";

            //
            // RFC 4785 - Pre-Shared Key (PSK) Ciphersuites with NULL Encryption
            //
            case TLS_PSK_WITH_NULL_SHA:
                return "PSK_WITH_NULL_SHA";
            case TLS_DHE_PSK_WITH_NULL_SHA:
                return "DHE_PSK_WITH_NULL_SHA";
            case TLS_RSA_PSK_WITH_NULL_SHA:
                return "RSA_PSK_WITH_NULL_SHA";

            //
            // Addenda from rfc 5288 AES Galois Counter Mode (GCM) Cipher Suites for TLS.
            //
            case TLS_RSA_WITH_AES_128_GCM_SHA256:
                return "RSA_WITH_AES_128_GCM_SHA256";
            case TLS_RSA_WITH_AES_256_GCM_SHA384:
                return "RSA_WITH_AES_256_GCM_SHA384";
            case TLS_DHE_RSA_WITH_AES_128_GCM_SHA256:
                return "DHE_RSA_WITH_AES_128_GCM_SHA256";
            case TLS_DHE_RSA_WITH_AES_256_GCM_SHA384:
                return "DHE_RSA_WITH_AES_256_GCM_SHA384";
            case TLS_DH_RSA_WITH_AES_128_GCM_SHA256:
                return "DH_RSA_WITH_AES_128_GCM_SHA256";
            case TLS_DH_RSA_WITH_AES_256_GCM_SHA384:
                return "DH_RSA_WITH_AES_256_GCM_SHA384";
            case TLS_DHE_DSS_WITH_AES_128_GCM_SHA256:
                return "DHE_DSS_WITH_AES_128_GCM_SHA256";
            case TLS_DHE_DSS_WITH_AES_256_GCM_SHA384:
                return "DHE_DSS_WITH_AES_256_GCM_SHA384";
            case TLS_DH_DSS_WITH_AES_128_GCM_SHA256:
                return "DH_DSS_WITH_AES_128_GCM_SHA256";
            case TLS_DH_DSS_WITH_AES_256_GCM_SHA384:
                return "DH_DSS_WITH_AES_256_GCM_SHA384";
            case TLS_DH_anon_WITH_AES_128_GCM_SHA256:
                return "DH_anon_WITH_AES_128_GCM_SHA256";
            case TLS_DH_anon_WITH_AES_256_GCM_SHA384:
                return "DH_anon_WITH_AES_256_GCM_SHA384";

            //
            // RFC 5487 - PSK with SHA-256/384 and AES GCM
            //
            case TLS_PSK_WITH_AES_128_GCM_SHA256:
                return "PSK_WITH_AES_128_GCM_SHA256";
            case TLS_PSK_WITH_AES_256_GCM_SHA384:
                return "PSK_WITH_AES_256_GCM_SHA384";
            case TLS_DHE_PSK_WITH_AES_128_GCM_SHA256:
                return "DHE_PSK_WITH_AES_128_GCM_SHA256";
            case TLS_DHE_PSK_WITH_AES_256_GCM_SHA384:
                return "DHE_PSK_WITH_AES_256_GCM_SHA384";
            case TLS_RSA_PSK_WITH_AES_128_GCM_SHA256:
                return "RSA_PSK_WITH_AES_128_GCM_SHA256";
            case TLS_RSA_PSK_WITH_AES_256_GCM_SHA384:
                return "RSA_PSK_WITH_AES_256_GCM_SHA384";

            case TLS_PSK_WITH_AES_128_CBC_SHA256:
                return "PSK_WITH_AES_128_CBC_SHA256";
            case TLS_PSK_WITH_AES_256_CBC_SHA384:
                return "PSK_WITH_AES_256_CBC_SHA384";
            case TLS_PSK_WITH_NULL_SHA256:
                return "WITH_NULL_SHA256";
            case TLS_PSK_WITH_NULL_SHA384:
                return "PSK_WITH_NULL_SHA384";

            case TLS_DHE_PSK_WITH_AES_128_CBC_SHA256:
                return "DHE_PSK_WITH_AES_128_CBC_SHA256";
            case TLS_DHE_PSK_WITH_AES_256_CBC_SHA384:
                return "DHE_PSK_WITH_AES_256_CBC_SHA384";
            case TLS_DHE_PSK_WITH_NULL_SHA256:
                return "DHE_PSK_WITH_NULL_SHA256";
            case TLS_DHE_PSK_WITH_NULL_SHA384:
                return "DHE_PSK_WITH_NULL_SHA384";

            case TLS_RSA_PSK_WITH_AES_128_CBC_SHA256:
                return "RSA_PSK_WITH_AES_128_CBC_SHA256";
            case TLS_RSA_PSK_WITH_AES_256_CBC_SHA384:
                return "RSA_PSK_WITH_AES_256_CBC_SHA384";
            case TLS_RSA_PSK_WITH_NULL_SHA256:
                return "RSA_PSK_WITH_NULL_SHA256";
            case TLS_RSA_PSK_WITH_NULL_SHA384:
                return "RSA_PSK_WITH_NULL_SHA384";

            //
            // Addenda from rfc 5289  Elliptic Curve Cipher Suites with HMAC SHA-256/384.
            //
            case TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256:
                return "ECDHE_ECDSA_WITH_AES_128_CBC_SHA256";
            case TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384:
                return "ECDHE_ECDSA_WITH_AES_256_CBC_SHA384";
            case TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256:
                return "ECDH_ECDSA_WITH_AES_128_CBC_SHA256";
            case TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384:
                return "ECDH_ECDSA_WITH_AES_256_CBC_SHA384";
            case TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256:
                return "ECDHE_RSA_WITH_AES_128_CBC_SHA256";
            case TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384:
                return "ECDHE_RSA_WITH_AES_256_CBC_SHA384";
            case TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256:
                return "ECDH_RSA_WITH_AES_128_CBC_SHA256";
            case TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384:
                return "ECDH_RSA_WITH_AES_256_CBC_SHA384";

            //
            // Addenda from rfc 5289  Elliptic Curve Cipher Suites with SHA-256/384 and AES Galois Counter Mode (GCM)
            //
            case TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256:
                return "ECDHE_ECDSA_WITH_AES_128_GCM_SHA256";
            case TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384:
                return "ECDHE_ECDSA_WITH_AES_256_GCM_SHA384";
            case TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256:
                return "ECDH_ECDSA_WITH_AES_128_GCM_SHA256";
            case TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384:
                return "ECDH_ECDSA_WITH_AES_256_GCM_SHA384";
            case TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256:
                return "ECDHE_RSA_WITH_AES_128_GCM_SHA256";
            case TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384:
                return "ECDHE_RSA_WITH_AES_256_GCM_SHA384";
            case TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256:
                return "ECDH_RSA_WITH_AES_128_GCM_SHA256";
            case TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384:
                return "ECDH_RSA_WITH_AES_256_GCM_SHA384";

            //
            // RFC 5746 - Secure Renegotiation
            //
            case TLS_EMPTY_RENEGOTIATION_INFO_SCSV:
                return "EMPTY_RENEGOTIATION_INFO_SCSV";

            //
            // Tags for SSL 2 cipher kinds that are not specified for SSL 3.
            //
            case SSL_RSA_WITH_RC2_CBC_MD5:
                return "RSA_WITH_RC2_CBC_MD5";
            case SSL_RSA_WITH_IDEA_CBC_MD5:
                return "RSA_WITH_IDEA_CBC_MD5";
            case SSL_RSA_WITH_DES_CBC_MD5:
                return "RSA_WITH_DES_CBC_MD5";
            case SSL_RSA_WITH_3DES_EDE_CBC_MD5:
                return "RSA_WITH_3DES_EDE_CBC_MD5";

            //
            // TLS 1.3 standard cipher suites
            //
            case TLS_AES_128_GCM_SHA256:
                return "TLS_AES_128_GCM_SHA256";
            case TLS_AES_256_GCM_SHA384:
                return "TLS_AES_256_GCM_SHA384";
            case TLS_CHACHA20_POLY1305_SHA256:
                return "TLS_CHACHA20_POLY1305_SHA256";
            case TLS_AES_128_CCM_SHA256:
                return "TLS_AES_128_CCM_SHA256";
            case TLS_AES_128_CCM_8_SHA256:
                return "TLS_AES_128_CCM_8_SHA256";

            default:
                return "";
        }
    }
}

IceSSL::SecureTransport::SSLEngine::SSLEngine(const IceInternal::InstancePtr& instance)
    : IceSSL::SSLEngine(instance),
      _certificateAuthorities(0),
      _chain(0)
{
}

//
// Setup the engine.
//
void
IceSSL::SecureTransport::SSLEngine::initialize()
{
    IceSSL::SSLEngine::initialize();

    const PropertiesPtr properties = getProperties();

    //
    // Check for a default directory. We look in this directory for
    // files mentioned in the configuration.
    //
    const string defaultDir = properties->getProperty("IceSSL.DefaultDir");

    //
    // Load the CA certificates used to authenticate peers into
    // _certificateAuthorities array.
    //
    try
    {
        string caFile = properties->getProperty("IceSSL.CAs");
        if (!caFile.empty())
        {
            string resolved;
            if (!checkPath(caFile, defaultDir, false, resolved))
            {
                throw InitializationException(__FILE__, __LINE__, "IceSSL: CA certificate file not found:\n" + caFile);
            }
            _certificateAuthorities.reset(loadCACertificates(resolved));
        }
        else if (properties->getPropertyAsInt("IceSSL.UsePlatformCAs") <= 0)
        {
            // Setup an empty list of Root CAs to not use the system root CAs.
            _certificateAuthorities.reset(CFArrayCreate(0, 0, 0, 0));
        }
    }
    catch (const CertificateReadException& ce)
    {
        throw InitializationException(__FILE__, __LINE__, ce.reason);
    }

    const string password = properties->getProperty("IceSSL.Password");

    string certFile = properties->getProperty("IceSSL.CertFile");
    string findCert = properties->getProperty("IceSSL.FindCert");
    string keychain = properties->getProperty("IceSSL.Keychain");
    string keychainPassword = properties->getProperty("IceSSL.KeychainPassword");

    if (!certFile.empty())
    {
        vector<string> files;
        if (!IceUtilInternal::splitString(certFile, IceUtilInternal::pathsep, files) || files.size() > 2)
        {
            throw InitializationException(
                __FILE__,
                __LINE__,
                "IceSSL: invalid value for IceSSL.CertFile:\n" + certFile);
        }
        vector<string> keyFiles;
        {
            string keyFile = properties->getProperty("IceSSL.KeyFile");
            if (!keyFile.empty())
            {
                if (!IceUtilInternal::splitString(keyFile, IceUtilInternal::pathsep, keyFiles) || keyFiles.size() > 2)
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: invalid value for IceSSL.KeyFile:\n" + keyFile);
                }
                if (files.size() != keyFiles.size())
                {
                    throw InitializationException(
                        __FILE__,
                        __LINE__,
                        "IceSSL: IceSSL.KeyFile does not agree with IceSSL.CertFile");
                }
            }
        }

        for (size_t i = 0; i < files.size(); ++i)
        {
            string file = files[i];
            string keyFile = keyFiles.empty() ? "" : keyFiles[i];
            string resolved;

            if (!checkPath(file, defaultDir, false, resolved))
            {
                throw InitializationException(__FILE__, __LINE__, "IceSSL: certificate file not found:\n" + file);
            }
            file = resolved;

            if (!keyFile.empty())
            {
                if (!checkPath(keyFile, defaultDir, false, resolved))
                {
                    throw InitializationException(__FILE__, __LINE__, "IceSSL: key file not found:\n" + keyFile);
                }
                keyFile = resolved;
            }

            try
            {
                _chain.reset(loadCertificateChain(file, keyFile, keychain, keychainPassword, password));
                break;
            }
            catch (const CertificateReadException& ce)
            {
                //
                // If this is the last certificate rethrow the exception as InitializationException,
                // otherwise try the next certificate.
                //
                if (i == files.size() - 1)
                {
                    throw InitializationException(__FILE__, __LINE__, ce.reason);
                }
            }
        }
    }
    else if (!findCert.empty())
    {
        _chain.reset(findCertificateChain(keychain, keychainPassword, findCert));
    }
}

//
// Destroy the engine.
//
void
IceSSL::SecureTransport::SSLEngine::destroy()
{
}

SSLContextRef
IceSSL::SecureTransport::SSLEngine::newContext(bool incoming)
{
    SSLContextRef ssl =
        SSLCreateContext(kCFAllocatorDefault, incoming ? kSSLServerSide : kSSLClientSide, kSSLStreamType);
    if (!ssl)
    {
        throw SecurityException(__FILE__, __LINE__, "IceSSL: unable to create SSL context");
    }

    OSStatus err = noErr;
    if (incoming)
    {
        switch (getVerifyPeer())
        {
            case 0:
            {
                SSLSetClientSideAuthenticate(ssl, kNeverAuthenticate);
                break;
            }
            case 1:
            {
                SSLSetClientSideAuthenticate(ssl, kTryAuthenticate);
                break;
            }
            case 2:
            {
                SSLSetClientSideAuthenticate(ssl, kAlwaysAuthenticate);
                break;
            }
            default:
            {
                assert(false);
                break;
            }
        }
    }

    if (_chain && (err = SSLSetCertificate(ssl, _chain.get())))
    {
        throw SecurityException(
            __FILE__,
            __LINE__,
            "IceSSL: error while setting the SSL context certificate:\n" + sslErrorToString(err));
    }

    if ((err = SSLSetSessionOption(
             ssl,
             incoming ? kSSLSessionOptionBreakOnClientAuth : kSSLSessionOptionBreakOnServerAuth,
             true)))
    {
        throw SecurityException(
            __FILE__,
            __LINE__,
            "IceSSL: error while setting SSL option:\n" + sslErrorToString(err));
    }

    return ssl;
}

CFArrayRef
IceSSL::SecureTransport::SSLEngine::getCertificateAuthorities() const
{
    return _certificateAuthorities.get();
}

string
IceSSL::SecureTransport::SSLEngine::getCipherName(SSLCipherSuite cipher) const
{
    return cipherName(cipher);
}
