// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

IceSSL::RSACertificateGenContext::RSACertificateGenContext() :
    _modulusLength(0),
    _secondsValid(0),
    _issuedAdjustment(0)
{
}

IceSSL::RSACertificateGenContext::~RSACertificateGenContext()
{
}

long
IceSSL::RSACertificateGenContext::minutesToSeconds(long minutes)
{
    return minutes * 60L;
}

long
IceSSL::RSACertificateGenContext::hoursToSeconds(long hours)
{
    return minutesToSeconds(hours * 60L);
}

long
IceSSL::RSACertificateGenContext::daysToSeconds(long days)
{
    return hoursToSeconds(days * 24L);
}

long
IceSSL::RSACertificateGenContext::weeksToSeconds(long weeks)
{
    return daysToSeconds(weeks * 7L);
}

long
IceSSL::RSACertificateGenContext::yearsToSeconds(long years)
{
    return weeksToSeconds(years * 365L);
}

void
IceSSL::RSACertificateGenContext::setCountry(const string& country)
{
    _country = country;
}

void
IceSSL::RSACertificateGenContext::setStateProvince(const string& stateProvince)
{
    _stateProvince = stateProvince;
}

void
IceSSL::RSACertificateGenContext::setLocality(const string& locality)
{
    _locality = locality;
}

void
IceSSL::RSACertificateGenContext::setOrganization(const string& organization)
{
    _organization = organization;
}

void
IceSSL::RSACertificateGenContext::setOrgainizationalUnit(const string& organizationalUnit)
{
    _organizationalUnit = organizationalUnit;
}

void
IceSSL::RSACertificateGenContext::setCommonName(const string& commonName)
{
    _commonName = commonName;
}

void
IceSSL::RSACertificateGenContext::setBitStrength(int bitStrength)
{
    _modulusLength = bitStrength;
}

void
IceSSL::RSACertificateGenContext::setSecondsValid(long secondsValid)
{
    _secondsValid = secondsValid;
}

void
IceSSL::RSACertificateGenContext::setIssuedAdjustment(long issuedAdjustment)
{
    _issuedAdjustment = issuedAdjustment;
}

unsigned char*
IceSSL::RSACertificateGenContext::getCountry() const
{
    unsigned char* country = reinterpret_cast<unsigned char *>(const_cast<char*>(_country.c_str()));

    assert(country != 0);

    return country;
}

unsigned char*
IceSSL::RSACertificateGenContext::getStateProvince() const
{
    unsigned char* stateProvince =  reinterpret_cast<unsigned char *>(const_cast<char*>(_stateProvince.c_str()));

    assert(stateProvince != 0);

    return stateProvince;
}

unsigned char*
IceSSL::RSACertificateGenContext::getLocality() const
{
    unsigned char* locality = reinterpret_cast<unsigned char *>(const_cast<char*>(_locality.c_str()));

    assert(locality != 0);

    return locality;
}

unsigned char*
IceSSL::RSACertificateGenContext::getOrganization() const
{
    unsigned char* organization = reinterpret_cast<unsigned char *>(const_cast<char*>(_organization.c_str()));

    assert(organization != 0);

    return organization;
}

unsigned char*
IceSSL::RSACertificateGenContext::getOrganizationalUnit() const
{
    unsigned char* orgUnit = reinterpret_cast<unsigned char *>(const_cast<char*>(_organizationalUnit.c_str()));

    assert(orgUnit != 0);

    return orgUnit;
}

unsigned char*
IceSSL::RSACertificateGenContext::getCommonName() const
{
    unsigned char* commonName = reinterpret_cast<unsigned char *>(const_cast<char*>(_commonName.c_str()));

    assert(commonName != 0);

    return commonName;
}

int
IceSSL::RSACertificateGenContext::getModulusLength() const
{
    return _modulusLength;
}

long
IceSSL::RSACertificateGenContext::getSecondsValid() const
{
    return _secondsValid;
}

long
IceSSL::RSACertificateGenContext::getIssuedAdjustment() const
{
    return _issuedAdjustment;
}

IceSSL::RSACertificateGen::RSACertificateGen()
{
    ERR_load_crypto_strings();
}

IceSSL::RSACertificateGen::~RSACertificateGen()
{
}

IceSSL::RSAKeyPairPtr
IceSSL::RSACertificateGen::generate(const RSACertificateGenContext& context)
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
#ifdef NDEBUG // Avoid compiler warnings when compiling with optimization.
    X509_set_version(x509SelfSigned, 2);
#else
    assert(X509_set_version(x509SelfSigned, 2) != 0);
#endif

    ASN1_INTEGER_set(X509_get_serialNumber(x509SelfSigned), 0);

    // NOTE: This is wierd.  It looks like, for some reason, that the typedef of
    // X509_NAME gets lost in this code module.  I am using the straight struct
    // here because X509_NAME isn't here.

    // X509_NAME* subjectName = X509_REQ_get_subject_name(signingRequest);
    struct X509_name_st* subjectName = X509_REQ_get_subject_name(signingRequest);

    // Set valid time period.
    X509_gmtime_adj(X509_get_notBefore(x509SelfSigned), context.getIssuedAdjustment());
    X509_gmtime_adj(X509_get_notAfter(x509SelfSigned), context.getSecondsValid());

    // Set up subject/issuer Distinguished Name (DN).
    X509_NAME_add_entry_by_txt(subjectName, const_cast<char*>("C"), MBSTRING_ASC, context.getCountry(),
        -1, -1, 0);
    X509_NAME_add_entry_by_txt(subjectName, const_cast<char*>("ST"), MBSTRING_ASC, context.getStateProvince(),
        -1, -1, 0);
    X509_NAME_add_entry_by_txt(subjectName, const_cast<char*>("L"), MBSTRING_ASC, context.getLocality(),
        -1, -1, 0);
    X509_NAME_add_entry_by_txt(subjectName, const_cast<char*>("O"), MBSTRING_ASC, context.getOrganization(),
        -1, -1, 0);
    X509_NAME_add_entry_by_txt(subjectName, const_cast<char*>("OU"), MBSTRING_ASC, context.getOrganizationalUnit(),
        -1, -1, 0);
    X509_NAME_add_entry_by_txt(subjectName, const_cast<char*>("CN"), MBSTRING_ASC, context.getCommonName(),
        -1, -1, 0);

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
    //             Checked this out, though, and there are no current issues (0.9.7a) with doing this.
    pkey->pkey.ptr = 0;

    RSAPrivateKeyPtr privKeyPtr = new RSAPrivateKey(rsaKeyPair);
    RSAPublicKeyPtr pubKeyPtr = new RSAPublicKey(x509SelfSigned);
    RSAKeyPair* keyPairPtr = new RSAKeyPair(privKeyPtr, pubKeyPtr);

    // Do not let the janitors clean up, we're keeping the keys for ourselves.
    rsaJanitor.clear();
    x509Janitor.clear();

    return keyPairPtr;
}

IceSSL::RSAKeyPairPtr
IceSSL::RSACertificateGen::loadKeyPair(const std::string& keyFile, const std::string& certFile)
{
    //
    // Read in the X509 Certificate Structure
    //
    BIOJanitor certBIO(BIO_new_file(certFile.c_str(), "r"));
    if(certBIO.get() == 0)
    {
        IceSSL::CertificateLoadException certLoadEx(__FILE__, __LINE__);

        certLoadEx.message = "unable to load certificate from '";
        certLoadEx.message += certFile;
        certLoadEx.message += "'\n";
        certLoadEx.message += sslGetErrors();

        throw certLoadEx;
    }

    X509Janitor x509Janitor(PEM_read_bio_X509(certBIO.get(), 0, 0, 0));

    if(x509Janitor.get() == 0)
    {
        IceSSL::CertificateLoadException certLoadEx(__FILE__, __LINE__);

        certLoadEx.message = "unable to load certificate from '";
        certLoadEx.message += certFile;
        certLoadEx.message += "'\n";
        certLoadEx.message += sslGetErrors();

        throw certLoadEx;
    }

    //
    // Read in the RSA Private Key Structure
    //
    BIOJanitor keyBIO(BIO_new_file(keyFile.c_str(), "r"));
    if(keyBIO.get() == 0)
    {
        IceSSL::PrivateKeyLoadException pklEx(__FILE__, __LINE__);

        pklEx.message = "unable to load private key from '";
        pklEx.message += keyFile;
        pklEx.message += "'\n";
        pklEx.message += sslGetErrors();

        throw pklEx;
    }

    RSAJanitor rsaJanitor(PEM_read_bio_RSAPrivateKey(keyBIO.get(), 0, 0, 0));

    if(rsaJanitor.get() == 0)
    {
        IceSSL::PrivateKeyLoadException pklEx(__FILE__, __LINE__);

        pklEx.message = "unable to load private key from '";
        pklEx.message += keyFile;
        pklEx.message += "'\n";
        pklEx.message += sslGetErrors();

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
