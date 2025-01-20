// Copyright (c) ZeroC, Inc.

#include "SecureTransportEngine.h"
#include "../FileUtil.h"
#include "../Instance.h"
#include "../TraceLevels.h"
#include "Ice/Config.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Logger.h"
#include "Ice/Properties.h"
#include "Ice/SSL/SSLException.h"
#include "SSLEngine.h"
#include "SSLUtil.h"
#include "SecureTransportUtil.h"

// Disable deprecation warnings from SecureTransport APIs
#include "../DisableWarnings.h"

using namespace std;
using namespace Ice;
using namespace Ice::SSL;
using namespace IceInternal;
using namespace Ice::SSL::SecureTransport;

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
                return "The requested name isn’t allowed for this certificate.";
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
}

SecureTransport::SSLEngine::SSLEngine(const IceInternal::InstancePtr& instance)
    : Ice::SSL::SSLEngine(instance),
      _certificateAuthorities(nullptr),
      _chain(nullptr)
{
}

SecureTransport::SSLEngine::~SSLEngine() = default;

//
// Setup the engine.
//
void
SecureTransport::SSLEngine::initialize()
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

//
// Destroy the engine.
//
void
SecureTransport::SSLEngine::destroy()
{
}

ClientAuthenticationOptions
SecureTransport::SSLEngine::createClientAuthenticationOptions(const string& host) const
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
SecureTransport::SSLEngine::createServerAuthenticationOptions() const
{
    SSLAuthenticate clientCertificateRequired;
    switch (getVerifyPeer())
    {
        case 0:
            clientCertificateRequired = kNeverAuthenticate;
            break;
        case 1:
            clientCertificateRequired = kTryAuthenticate;
            break;
        default:
            clientCertificateRequired = kAlwaysAuthenticate;
            break;
    }

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
        .clientCertificateRequired = clientCertificateRequired,
        .trustedRootCertificates = _certificateAuthorities.get(),
        .clientCertificateValidationCallback = [this](SecTrustRef trust, const Ice::SSL::ConnectionInfoPtr& info)
        { return validationCallback(trust, info, ""); }};
}

SSLContextRef
SecureTransport::SSLEngine::newContext(bool incoming) const
{
    SSLContextRef ssl =
        SSLCreateContext(kCFAllocatorDefault, incoming ? kSSLServerSide : kSSLClientSide, kSSLStreamType);
    if (!ssl)
    {
        throw SecurityException(__FILE__, __LINE__, "SSL transport: unable to create SSL context");
    }

    OSStatus err = SSLSetSessionOption(
        ssl,
        incoming ? kSSLSessionOptionBreakOnClientAuth : kSSLSessionOptionBreakOnServerAuth,
        true);

    if (err != noErr)
    {
        throw SecurityException(
            __FILE__,
            __LINE__,
            "SSL transport: error while setting SSL option:\n" + sslErrorToString(err));
    }

    return ssl;
}

bool
SecureTransport::SSLEngine::validationCallback(SecTrustRef trust, const ConnectionInfoPtr& info, const string& host)
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
    verifyPeer(info);
    return true;
}

string
SecureTransport::SSLEngine::getCipherName(SSLCipherSuite cipher) const
{
    return cipherName(cipher);
}
