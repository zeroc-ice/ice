//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "../Ice/ProtocolInstance.h"
#include "../Ice/ProtocolPluginFacade.h"
#include "Ice/LocalException.h"
#include "Ice/RegisterPlugins.h"
#include "IceSSL/Certificate.h"
#include "SSLEndpointI.h"
#include "SSLEngine.h"
#include "SSLInstance.h"

using namespace std;
using namespace Ice;
using namespace IceSSL;

std::string
IceSSL::getTrustErrorDescription(TrustError error)
{
    switch (error)
    {
        case IceSSL::TrustError::NoError:
        {
            return "no error";
        }
        case IceSSL::TrustError::ChainTooLong:
        {
            return "the certificate chain length is greater than the specified maximum depth";
        }
        case IceSSL::TrustError::HasExcludedNameConstraint:
        {
            return "the X509 chain is invalid because a certificate has excluded a name constraint";
        }
        case IceSSL::TrustError::HasNonDefinedNameConstraint:
        {
            return "the certificate has an undefined name constraint";
        }
        case IceSSL::TrustError::HasNonPermittedNameConstraint:
        {
            return "the certificate has a non permitted name constrain";
        }
        case IceSSL::TrustError::HasNonSupportedCriticalExtension:
        {
            return "the certificate does not support a critical extension";
        }
        case IceSSL::TrustError::HasNonSupportedNameConstraint:
        {
            return "the certificate does not have a supported name constraint or has a name constraint that "
                   "is unsupported";
        }
        case IceSSL::TrustError::HostNameMismatch:
        {
            return "a host name mismatch has occurred";
        }
        case IceSSL::TrustError::InvalidBasicConstraints:
        {
            return "the X509 chain is invalid due to invalid basic constraints";
        }
        case IceSSL::TrustError::InvalidExtension:
        {
            return "the X509 chain is invalid due to an invalid extension";
        }
        case IceSSL::TrustError::InvalidNameConstraints:
        {
            return "the X509 chain is invalid due to invalid name constraints";
        }
        case IceSSL::TrustError::InvalidPolicyConstraints:
        {
            return "the X509 chain is invalid due to invalid policy constraints";
        }
        case IceSSL::TrustError::InvalidPurpose:
        {
            return "the supplied certificate cannot be used for the specified purpose";
        }
        case IceSSL::TrustError::InvalidSignature:
        {
            return "the X509 chain is invalid due to an invalid certificate signature";
        }
        case IceSSL::TrustError::InvalidTime:
        {
            return "the X509 chain is not valid due to an invalid time value, such as a value that indicates an "
                   "expired certificate";
        }
        case IceSSL::TrustError::NotTrusted:
        {
            return "the certificate is explicitly distrusted";
        }
        case IceSSL::TrustError::PartialChain:
        {
            return "the X509 chain could not be built up to the root certificate";
        }
        case IceSSL::TrustError::RevocationStatusUnknown:
        {
            return "it is not possible to determine whether the certificate has been revoked";
        }
        case IceSSL::TrustError::Revoked:
        {
            return "the X509 chain is invalid due to a revoked certificate";
        }
        case IceSSL::TrustError::UntrustedRoot:
        {
            return "the X509 chain is invalid due to an untrusted root certificate";
        }
        case IceSSL::TrustError::UnknownTrustFailure:
        {
            return "unknown failure";
        }
    }
    assert(false);
    return "unknown failure";
}
