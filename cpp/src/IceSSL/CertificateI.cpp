//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "CertificateI.h"
#include "../Ice/Base64.h"
#include "Ice/LocalException.h"
#include "Ice/Object.h"
#include "Ice/StringConverter.h"
#include "IceUtil/DisableWarnings.h"
#include "IceUtil/StringUtil.h"
#include "RFC2253.h"
#include "SSLUtil.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceSSL;

//
// Map a certificate OID to its alias
//
const CertificateOID IceSSL::certificateOIDS[] = {
    {"2.5.4.3", "CN"},
    {"2.5.4.4", "SN"},
    {"2.5.4.5", "DeviceSerialNumber"},
    {"2.5.4.6", "C"},
    {"2.5.4.7", "L"},
    {"2.5.4.8", "ST"},
    {"2.5.4.9", "STREET"},
    {"2.5.4.10", "O"},
    {"2.5.4.11", "OU"},
    {"2.5.4.12", "T"},
    {"2.5.4.42", "G"},
    {"2.5.4.43", "I"},
    {"1.2.840.113549.1.9.8", "unstructuredAddress"},
    {"1.2.840.113549.1.9.2", "unstructuredName"},
    {"1.2.840.113549.1.9.1", "emailAddress"},
    {"0.9.2342.19200300.100.1.25", "DC"}};
const int IceSSL::certificateOIDSSize = sizeof(IceSSL::certificateOIDS) / sizeof(CertificateOID);

CertificateReadException::CertificateReadException(const char* file, int line, string r) noexcept
    : Exception(file, line),
      reason(std::move(r))
{
}

string
CertificateReadException::ice_id() const
{
    return "::IceSSL::CertificateReadException";
}

CertificateEncodingException::CertificateEncodingException(const char* file, int line, string r) noexcept
    : Exception(file, line),
      reason(std::move(r))
{
}

string
CertificateEncodingException::ice_id() const
{
    return "::IceSSL::CertificateEncodingException";
}

ParseException::ParseException(const char* file, int line, string r) noexcept
    : Exception(file, line),
      reason(std::move(r))
{
}

string
ParseException::ice_id() const
{
    return "::IceSSL::ParseException";
}

DistinguishedName::DistinguishedName(const string& dn) : _rdns(RFC2253::parseStrict(dn)) { unescape(); }

DistinguishedName::DistinguishedName(const list<pair<string, string>>& rdns) : _rdns(rdns) { unescape(); }

namespace IceSSL
{
    bool operator==(const DistinguishedName& lhs, const DistinguishedName& rhs)
    {
        return lhs._unescaped == rhs._unescaped;
    }

    bool operator<(const DistinguishedName& lhs, const DistinguishedName& rhs)
    {
        return lhs._unescaped < rhs._unescaped;
    }
}

bool
DistinguishedName::match(const DistinguishedName& other) const
{
    for (list<pair<string, string>>::const_iterator p = other._unescaped.begin(); p != other._unescaped.end(); ++p)
    {
        bool found = false;
        for (list<pair<string, string>>::const_iterator q = _unescaped.begin(); q != _unescaped.end(); ++q)
        {
            if (p->first == q->first)
            {
                found = true;
                if (p->second != q->second)
                {
                    return false;
                }
            }
        }
        if (!found)
        {
            return false;
        }
    }
    return true;
}

bool
DistinguishedName::match(const string& other) const
{
    return match(DistinguishedName(other));
}

//
// This always produces the same output as the input DN -- the type of
// escaping is not changed.
//
std::string
DistinguishedName::toString() const
{
    ostringstream os;
    bool first = true;
    for (list<pair<string, string>>::const_iterator p = _rdns.begin(); p != _rdns.end(); ++p)
    {
        if (!first)
        {
            os << ",";
        }
        first = false;
        os << p->first << "=" << p->second;
    }
    return os.str();
}

void
DistinguishedName::unescape()
{
    for (list<pair<string, string>>::const_iterator q = _rdns.begin(); q != _rdns.end(); ++q)
    {
        pair<string, string> rdn = *q;
        rdn.second = RFC2253::unescape(rdn.second);
        _unescaped.push_back(rdn);
    }
}

bool
CertificateI::operator!=(const IceSSL::Certificate& other) const
{
    return !operator==(other);
}

vector<X509ExtensionPtr>
CertificateI::getX509Extensions() const
{
    loadX509Extensions(); // Lazzy initialize the extensions
    return _extensions;
}

X509ExtensionPtr
CertificateI::getX509Extension(const string& oid) const
{
    loadX509Extensions(); // Lazzy initialize the extensions
    X509ExtensionPtr ext;
    for (vector<X509ExtensionPtr>::const_iterator i = _extensions.begin(); i != _extensions.end(); ++i)
    {
        if ((*i)->getOID() == oid)
        {
            ext = *i;
            break;
        }
    }
    return ext;
}

void
CertificateI::loadX509Extensions() const
{
    throw FeatureNotSupportedException(__FILE__, __LINE__);
}

bool
CertificateI::checkValidity() const
{
    auto now = chrono::system_clock::now();
    return now > getNotBefore() && now <= getNotAfter();
}

bool
CertificateI::checkValidity(const chrono::system_clock::time_point& now) const
{
    return now > getNotBefore() && now <= getNotAfter();
}

string
CertificateI::toString() const
{
    ostringstream os;
    os << "serial: " << getSerialNumber() << "\n";
    os << "issuer: " << string(getIssuerDN()) << "\n";
    os << "subject: " << string(getSubjectDN()) << "\n";
    return os.str();
}

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
