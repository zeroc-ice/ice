// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Config.h>
#include <IceSSL/RSACertificateGen.h>
#include <IceSSL/OpenSSLJanitors.h>
#include <IceSSL/RSAKeyPair.h>
#include <IceSSL/RSAPrivateKey.h>
#include <IceSSL/RSAPublicKey.h>
#include <IceSSL/Exception.h>
#include <IceSSL/OpenSSLUtils.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

using std::string;
using std::back_inserter;

using namespace IceSSL::OpenSSL;

long
IceSSL::OpenSSL::RSACertificateGenContext::minutesToSeconds(long minutes)
{
    return minutes * 60L;
}

long
IceSSL::OpenSSL::RSACertificateGenContext::hoursToSeconds(long hours)
{
    return minutesToSeconds(hours * 60L);
}

long
IceSSL::OpenSSL::RSACertificateGenContext::daysToSeconds(long days)
{
    return hoursToSeconds(days * 24L);
}

long
IceSSL::OpenSSL::RSACertificateGenContext::weeksToSeconds(long weeks)
{
    return daysToSeconds(weeks * 7L);
}

long
IceSSL::OpenSSL::RSACertificateGenContext::yearsToSeconds(long years)
{
    return weeksToSeconds(years * 365L);
}

IceSSL::OpenSSL::RSACertificateGenContext::RSACertificateGenContext() :
                                          _modulusLength(0),
                                          _secondsValid(0)
{
}

IceSSL::OpenSSL::RSACertificateGenContext::~RSACertificateGenContext()
{
}

void
IceSSL::OpenSSL::RSACertificateGenContext::setCountry(const string& country)
{
    _country = country;
}

void
IceSSL::OpenSSL::RSACertificateGenContext::setStateProvince(const string& stateProvince)
{
    _stateProvince = stateProvince;
}

void
IceSSL::OpenSSL::RSACertificateGenContext::setLocality(const string& locality)
{
    _locality = locality;
}

void
IceSSL::OpenSSL::RSACertificateGenContext::setOrganization(const string& organization)
{
    _organization = organization;
}

void
IceSSL::OpenSSL::RSACertificateGenContext::setOrgainizationalUnit(const string& organizationalUnit)
{
    _organizationalUnit = organizationalUnit;
}

void
IceSSL::OpenSSL::RSACertificateGenContext::setCommonName(const string& commonName)
{
    _commonName = commonName;
}

void
IceSSL::OpenSSL::RSACertificateGenContext::setBitStrength(int bitStrength)
{
    _modulusLength = bitStrength;
}

void
IceSSL::OpenSSL::RSACertificateGenContext::setSecondsValid(long secondsValid)
{
    _secondsValid = secondsValid;
}

unsigned char*
IceSSL::OpenSSL::RSACertificateGenContext::getCountry() const
{
    unsigned char* country = reinterpret_cast<unsigned char *>(const_cast<char*>(_country.c_str()));

    assert(country != 0);

    return country;
}

unsigned char*
IceSSL::OpenSSL::RSACertificateGenContext::getStateProvince() const
{
    unsigned char* stateProvince =  reinterpret_cast<unsigned char *>(const_cast<char*>(_stateProvince.c_str()));

    assert(stateProvince != 0);

    return stateProvince;
}

unsigned char*
IceSSL::OpenSSL::RSACertificateGenContext::getLocality() const
{
    unsigned char* locality = reinterpret_cast<unsigned char *>(const_cast<char*>(_locality.c_str()));

    assert(locality != 0);

    return locality;
}

unsigned char*
IceSSL::OpenSSL::RSACertificateGenContext::getOrganization() const
{
    unsigned char* organization = reinterpret_cast<unsigned char *>(const_cast<char*>(_organization.c_str()));

    assert(organization != 0);

    return organization;
}

unsigned char*
IceSSL::OpenSSL::RSACertificateGenContext::getOrgainizationalUnit() const
{
    unsigned char* orgUnit = reinterpret_cast<unsigned char *>(const_cast<char*>(_organizationalUnit.c_str()));

    assert(orgUnit != 0);

    return orgUnit;
}

unsigned char*
IceSSL::OpenSSL::RSACertificateGenContext::getCommonName() const
{
    unsigned char* commonName = reinterpret_cast<unsigned char *>(const_cast<char*>(_commonName.c_str()));

    assert(commonName != 0);

    return commonName;
}

int
IceSSL::OpenSSL::RSACertificateGenContext::getModulusLength() const
{
    return _modulusLength;
}

long
IceSSL::OpenSSL::RSACertificateGenContext::getSecondsValid() const
{
    return _secondsValid;
}

IceSSL::OpenSSL::RSACertificateGen::RSACertificateGen()
{
    ERR_load_crypto_strings();
}

IceSSL::OpenSSL::RSACertificateGen::~RSACertificateGen()
{
}

IceSSL::OpenSSL::RSAKeyPairPtr
IceSSL::OpenSSL::RSACertificateGen::generate(const RSACertificateGenContext& context)
{
    // Generate an RSA key pair.
    RSAJanitor rsaJanitor(RSA_generate_key(context.getModulusLength(), RSA_F4, 0, 0));
    RSA* rsaKeyPair = rsaJanitor.get();

    assert(rsaKeyPair != 0);

    EVP_PKEYJanitor evpPkeyJanitor(EVP_PKEY_new());
    EVP_PKEY* pkey = evpPkeyJanitor.get();
    assert(pkey != 0);
    EVP_PKEY_assign_RSA(pkey, rsaKeyPair);

    // The RSA structure now belongs (temporarily) to the EVP_PKEY
    rsaJanitor.clear();

    // Create a signing request
    X509_REQJanitor x509ReqJanitor(X509_REQ_new());
    X509_REQ* signingRequest = x509ReqJanitor.get();
    assert(signingRequest != 0);

    X509Janitor x509Janitor(X509_new());
    X509* x509SelfSigned = x509Janitor.get();
    assert(x509SelfSigned != 0);

    // Set version to V3.
    int setVersionReturn = X509_set_version(x509SelfSigned, 2);
    assert(setVersionReturn != 0);

    ASN1_INTEGER_set(X509_get_serialNumber(x509SelfSigned), 0);

    // NOTE: This is wierd.  It looks like, for some reason, that the typedef of
    // X509_NAME gets lost in this code module.  I am using the straight struct
    // here because X509_NAME isn't here.

    // X509_NAME* subjectName = X509_REQ_get_subject_name(signingRequest);
    struct X509_name_st* subjectName = X509_REQ_get_subject_name(signingRequest);

    // Set valid time period.
    X509_gmtime_adj(X509_get_notBefore(x509SelfSigned), 0);
    X509_gmtime_adj(X509_get_notAfter(x509SelfSigned), context.getSecondsValid());

    // Set up subject/issuer Distinguished Name (DN).
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

    // Sign the public key using an MD5 digest.
    if(!X509_sign(x509SelfSigned, pkey, EVP_md5()))
    {
        throw IceSSL::CertificateSigningException(__FILE__, __LINE__);
    }

    // Verify the Signature (paranoia).
    if(!X509_REQ_verify(signingRequest, pkey))
    {
        throw IceSSL::CertificateSignatureException(__FILE__, __LINE__);
    }

    // Nasty Hack: Getting the pkey to let go of our rsaKeyPair - we own that now.
    pkey->pkey.ptr = 0;

    RSAPrivateKeyPtr privKeyPtr = new RSAPrivateKey(rsaKeyPair);
    RSAPublicKeyPtr pubKeyPtr = new RSAPublicKey(x509SelfSigned);
    RSAKeyPair* keyPairPtr = new RSAKeyPair(privKeyPtr, pubKeyPtr);

    // Do not let the janitors clean up, we're keeping the keys for ourselves.
    rsaJanitor.clear();
    x509Janitor.clear();

    return keyPairPtr;
}

IceSSL::OpenSSL::RSAKeyPairPtr
IceSSL::OpenSSL::RSACertificateGen::loadKeyPair(const std::string& keyFile, const std::string& certFile)
{
    //
    // Read in the X509 Certificate Structure
    //
    BIOJanitor certBIO(BIO_new_file(certFile.c_str(), "r"));
    if(certBIO.get() == 0)
    {
        IceSSL::OpenSSL::CertificateLoadException certLoadEx(__FILE__, __LINE__);

        certLoadEx._message = "unable to load certificate from '";
        certLoadEx._message += certFile;
        certLoadEx._message += "'\n";
        certLoadEx._message += sslGetErrors();

        throw certLoadEx;
    }

    X509Janitor x509Janitor(PEM_read_bio_X509(certBIO.get(), 0, 0, 0));

    if(x509Janitor.get() == 0)
    {
        IceSSL::OpenSSL::CertificateLoadException certLoadEx(__FILE__, __LINE__);

        certLoadEx._message = "unable to load certificate from '";
        certLoadEx._message += certFile;
        certLoadEx._message += "'\n";
        certLoadEx._message += sslGetErrors();

        throw certLoadEx;
    }

    //
    // Read in the RSA Private Key Structure
    //
    BIOJanitor keyBIO(BIO_new_file(keyFile.c_str(), "r"));
    if(keyBIO.get() == 0)
    {
        IceSSL::OpenSSL::PrivateKeyLoadException pklEx(__FILE__, __LINE__);

        pklEx._message = "unable to load private key from '";
        pklEx._message += keyFile;
        pklEx._message += "'\n";
        pklEx._message += sslGetErrors();

        throw pklEx;
    }

    RSAJanitor rsaJanitor(PEM_read_bio_RSAPrivateKey(keyBIO.get(), 0, 0, 0));

    if(rsaJanitor.get() == 0)
    {
        IceSSL::OpenSSL::PrivateKeyLoadException pklEx(__FILE__, __LINE__);

        pklEx._message = "unable to load private key from '";
        pklEx._message += keyFile;
        pklEx._message += "'\n";
        pklEx._message += sslGetErrors();

        throw pklEx;
    }

    //
    // Construct our RSAKeyPair
    //
    RSAPrivateKeyPtr privKeyPtr = new RSAPrivateKey(rsaJanitor.get());
    RSAPublicKeyPtr pubKeyPtr = new RSAPublicKey(x509Janitor.get());
    RSAKeyPairPtr keyPairPtr = new RSAKeyPair(privKeyPtr, pubKeyPtr);

    // Do not let the janitors clean up, we're keeping these keys.
    rsaJanitor.clear();
    x509Janitor.clear();

    return keyPairPtr;
}
