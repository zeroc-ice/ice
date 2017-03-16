// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>
#include <IceUtil/StringUtil.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/Util.h>
#include <IceSSL/RFC2253.h>
#include <IceSSL/CertificateI.h>
#include <Ice/Object.h>
#include <Ice/Base64.h>
#include <Ice/StringConverter.h>
#include <IceUtil/Time.h>


using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceSSL;

//
// Map a certificate OID to its alias
//
const CertificateOID IceSSL::certificateOIDS[] =
{
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
    {"0.9.2342.19200300.100.1.25", "DC"}
};
const int IceSSL::certificateOIDSSize = sizeof(IceSSL::certificateOIDS) / sizeof(CertificateOID);

CertificateReadException::CertificateReadException(const char* file, int line, const string& r) :
    ExceptionHelper<CertificateReadException>(file, line),
    reason(r)
{
}

#ifndef ICE_CPP11_COMPILER
CertificateReadException::~CertificateReadException() throw()
{
}
#endif

string
CertificateReadException::ice_id() const
{
    return "::IceSSL::CertificateReadException";
}

#ifndef ICE_CPP11_MAPPING
CertificateReadException*
CertificateReadException::ice_clone() const
{
    return new CertificateReadException(*this);
}
#endif

CertificateEncodingException::CertificateEncodingException(const char* file, int line, const string& r) :
    ExceptionHelper<CertificateEncodingException>(file, line),
    reason(r)
{
}

#ifndef ICE_CPP11_COMPILER
CertificateEncodingException::~CertificateEncodingException() throw()
{
}
#endif

string
CertificateEncodingException::ice_id() const
{
    return "::IceSSL::CertificateEncodingException";
}

#ifndef ICE_CPP11_MAPPING
CertificateEncodingException*
CertificateEncodingException::ice_clone() const
{
    return new CertificateEncodingException(*this);
}
#endif

ParseException::ParseException(const char* file, int line, const string& r) :
    ExceptionHelper<ParseException>(file, line),
    reason(r)
{
}

#ifndef ICE_CPP11_COMPILER
ParseException::~ParseException() throw()
{
}
#endif

string
ParseException::ice_id() const
{
    return "::IceSSL::ParseException";
}

#ifndef ICE_CPP11_MAPPING
ParseException*
ParseException::ice_clone() const
{
    return new ParseException(*this);
}
#endif

DistinguishedName::DistinguishedName(const string& dn) : _rdns(RFC2253::parseStrict(dn))
{
    unescape();
}

DistinguishedName::DistinguishedName(const list<pair<string, string> >& rdns) : _rdns(rdns)
{
    unescape();
}

namespace IceSSL
{

bool
operator==(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return lhs._unescaped == rhs._unescaped;
}

bool
operator<(const DistinguishedName& lhs, const DistinguishedName& rhs)
{
    return lhs._unescaped < rhs._unescaped;
}

}

bool
DistinguishedName::match(const DistinguishedName& other) const
{
    for(list< pair<string, string> >::const_iterator p = other._unescaped.begin(); p != other._unescaped.end(); ++p)
    {
        bool found = false;
        for(list< pair<string, string> >::const_iterator q = _unescaped.begin(); q != _unescaped.end(); ++q)
        {
            if(p->first == q->first)
            {
                found = true;
                if(p->second != q->second)
                {
                    return false;
                }
            }
        }
        if(!found)
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
DistinguishedName::operator string() const
{
    ostringstream os;
    bool first = true;
    for(list< pair<string, string> >::const_iterator p = _rdns.begin(); p != _rdns.end(); ++p)
    {
        if(!first)
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
    for(list< pair<string, string> >::const_iterator q = _rdns.begin(); q != _rdns.end(); ++q)
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
    for(vector<X509ExtensionPtr>::const_iterator i = _extensions.begin(); i != _extensions.end(); ++i)
    {
        if((*i)->getOID() == oid)
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
#  ifdef ICE_CPP11_MAPPING
    auto now = chrono::system_clock::now();
#  else
    IceUtil::Time now = IceUtil::Time::now();
#  endif
    return now > getNotBefore() && now <= getNotAfter();
}

bool
#  ifdef ICE_CPP11_MAPPING
CertificateI::checkValidity(const chrono::system_clock::time_point& now) const
#  else
CertificateI::checkValidity(const IceUtil::Time& now) const
#  endif
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
