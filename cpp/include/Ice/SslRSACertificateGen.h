// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CERTIFICATE_GEN_H
#define ICE_CERTIFICATE_GEN_H

#include <vector>
#include <string>
#include <Ice/SslRSAKeyPairF.h>

#ifdef WIN32
#   ifdef ICE_API_EXPORTS
#       define ICE_API __declspec(dllexport)
#   else
#       define ICE_API __declspec(dllimport)
#   endif
#else
#   define ICE_API /**/
#endif

namespace IceSecurity
{

namespace Ssl
{

namespace OpenSSL
{

using namespace std;

class ICE_API RSACertificateGenContext
{

public:

    //
    // Static
    //

    // Conversion helper functions
    static long minutesToSeconds(long);
    static long hoursToSeconds(long);
    static long daysToSeconds(long);
    static long weeksToSeconds(long);
    static long yearsToSeconds(long);

    //
    // Instance
    //

    RSACertificateGenContext();
    ~RSACertificateGenContext();

    // Distinguished Name methods (setters)
    void setCountry(const std::string&);
    void setStateProvince(const std::string&);
    void setLocality(const std::string&);
    void setOrganization(const std::string&);
    void setOrgainizationalUnit(const std::string&);
    void setCommonName(const std::string&);

    // Setters
    void setBitStrength(int);
    void setSecondsValid(long);

    // Distinguished Name methods (getters)
    unsigned char* getCountry() const;
    unsigned char* getStateProvince() const;
    unsigned char* getLocality() const;
    unsigned char* getOrganization() const;
    unsigned char* getOrgainizationalUnit() const;
    unsigned char* getCommonName() const;

    // Getters
    int getModulusLength() const;
    long getSecondsValid() const;


private:
    std::string _country;
    std::string _stateProvince;
    std::string _locality;
    std::string _organization;
    std::string _organizationalUnit;
    std::string _commonName;
    int _modulusLength;
    long _secondsValid;
};

class ICE_API RSACertificateGen
{

public:
    RSACertificateGen();
    ~RSACertificateGen();

    RSAKeyPair* generate(const RSACertificateGenContext&);

};

}

}

}

#endif
