// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Config.h>
#include <Ice/SslRSACertificateGen.h>
#include <Ice/SslJanitors.h>
#include <Ice/SslRSAKeyPair.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

using std::string;
using std::back_inserter;

using namespace IceSecurity::Ssl::OpenSSL;

long
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::minutesToSeconds(long minutes)
{
    return minutes * 60L;
}

long
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::hoursToSeconds(long hours)
{
    return minutesToSeconds(hours * 60L);
}

long
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::daysToSeconds(long days)
{
    return hoursToSeconds(days * 24L);
}

long
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::weeksToSeconds(long weeks)
{
    return daysToSeconds(weeks * 7L);
}

long
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::yearsToSeconds(long years)
{
    return weeksToSeconds(years * 365L);
}

IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::RSACertificateGenContext() :
                                                 _modulusLength(0),
                                                 _secondsValid(0)
{
}

void
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::setCountry(const string& country)
{
    _country = country;
}

void
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::setStateProvince(const string& stateProvince)
{
    _stateProvince = stateProvince;
}

void
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::setLocality(const string& locality)
{
    _locality = locality;
}

void
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::setOrganization(const string& organization)
{
    _organization = organization;
}

void
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::setOrgainizationalUnit(const string& organizationalUnit)
{
    _organizationalUnit = organizationalUnit;
}

void
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::setCommonName(const string& commonName)
{
    _commonName = commonName;
}

void
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::setBitStrength(int bitStrength)
{
    _modulusLength = bitStrength;
}

void
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::setSecondsValid(long secondsValid)
{
    _secondsValid = secondsValid;
}

unsigned char*
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::getCountry() const
{
    return reinterpret_cast<unsigned char *>(const_cast<char*>(_country.c_str()));
}

unsigned char*
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::getStateProvince() const
{
    return reinterpret_cast<unsigned char *>(const_cast<char*>(_stateProvince.c_str()));
}

unsigned char*
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::getLocality() const
{
    return reinterpret_cast<unsigned char *>(const_cast<char*>(_locality.c_str()));
}

unsigned char*
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::getOrganization() const
{
    return reinterpret_cast<unsigned char *>(const_cast<char*>(_organization.c_str()));
}

unsigned char*
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::getOrgainizationalUnit() const
{
    return reinterpret_cast<unsigned char *>(const_cast<char*>(_organizationalUnit.c_str()));
}

unsigned char*
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::getCommonName() const
{
    return reinterpret_cast<unsigned char *>(const_cast<char*>(_commonName.c_str()));
}

int
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::getModulusLength() const
{
    return _modulusLength;
}

long
IceSecurity::Ssl::OpenSSL::RSACertificateGenContext::getSecondsValid() const
{
    return _secondsValid;
}

IceSecurity::Ssl::OpenSSL::RSACertificateGen::RSACertificateGen()
{
    ERR_load_crypto_strings();
}

IceSecurity::Ssl::OpenSSL::RSACertificateGen::~RSACertificateGen()
{
}

IceSecurity::Ssl::OpenSSL::RSAKeyPair*
IceSecurity::Ssl::OpenSSL::RSACertificateGen::generate(const RSACertificateGenContext& context)
{
    // Generate an RSA key pair.
    RSAJanitor rsaJanitor(RSA_generate_key(context.getModulusLength(), RSA_F4, 0, 0));
    RSA* rsaKeyPair = rsaJanitor.get();

    assert(rsaKeyPair);

    // Do this if we already have an RSA* 
    EVP_PKEYJanitor evpPkeyJanitor(EVP_PKEY_new());
    EVP_PKEY* pkey = evpPkeyJanitor.get();
    assert(pkey);
    EVP_PKEY_assign_RSA(pkey, rsaKeyPair);

    // The RSA structure now belongs (temporarily) to the EVP_PKEY
    rsaJanitor.clear();

    // Create a signing request
    X509_REQJanitor x509ReqJanitor(X509_REQ_new());
    X509_REQ* signingRequest = x509ReqJanitor.get();
    assert(signingRequest);

    X509Janitor x509Janitor(X509_new());
    X509* x509SelfSigned = x509Janitor.get();
    assert(x509SelfSigned);

    // Set version to V3
    assert(X509_set_version(x509SelfSigned, 2));

    ASN1_INTEGER_set(X509_get_serialNumber(x509SelfSigned), 0);

    // NOTE: This is wierd.  It looks like, for some reason, that the typedef of
    // X509_NAME gets lost in this code module.  I am using the straight struct
    // here because X509_NAME isn't here.

    // X509_NAME* subjectName = X509_REQ_get_subject_name(signingRequest);
    struct X509_name_st* subjectName = X509_REQ_get_subject_name(signingRequest);

    // Set valid time period
    X509_gmtime_adj(X509_get_notBefore(x509SelfSigned), 0);
    X509_gmtime_adj(X509_get_notAfter(x509SelfSigned), context.getSecondsValid());

    // Set up subject/issuer name
    X509_NAME_add_entry_by_txt(subjectName, "C",  MBSTRING_ASC, context.getCountry(),             -1, -1, 0);
    X509_NAME_add_entry_by_txt(subjectName, "ST", MBSTRING_ASC, context.getStateProvince(),       -1, -1, 0);
    X509_NAME_add_entry_by_txt(subjectName, "L",  MBSTRING_ASC, context.getLocality(),            -1, -1, 0);
    X509_NAME_add_entry_by_txt(subjectName, "O",  MBSTRING_ASC, context.getOrganization(),        -1, -1, 0);
    X509_NAME_add_entry_by_txt(subjectName, "OU", MBSTRING_ASC, context.getOrgainizationalUnit(), -1, -1, 0);
    X509_NAME_add_entry_by_txt(subjectName, "CN", MBSTRING_ASC, context.getCommonName(),          -1, -1, 0);

    // Self signed - set issuer and subject names identical
    X509_set_issuer_name(x509SelfSigned, subjectName);
    X509_set_subject_name(x509SelfSigned, subjectName);

    // Set the public key in the self signed certificate from the request.
    X509_set_pubkey(x509SelfSigned, pkey);

    // Sign the public key using an MD5 digest
    if (!X509_sign(x509SelfSigned, pkey, EVP_md5()))
    {
        // TODO: Throw exception here.
        // throw CertificateSigningException(__FILE__, __LINE__);
        return 0;
    }

    // Verify the Signature (paranoia)
    if (!X509_REQ_verify(signingRequest, pkey))
    {
        // TODO: Throw exception here (signature verification)
        // throw CertificateSignatureException(__FILE__, __LINE__);
        return 0;
    }

    // Nasty Hack: Getting the pkey to let go of our rsaKeyPair - we own that.
    pkey->pkey.ptr = 0;

    // Constructing our object.
    RSAKeyPair* keyPairPtr = new RSAKeyPair(rsaKeyPair, x509SelfSigned);

    // Don't let them clean up, we're keeping those around.
    rsaJanitor.clear();
    x509Janitor.clear();

    return keyPairPtr;
}




