// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_GENERAL_CONFIG_H
#define ICE_SSL_GENERAL_CONFIG_H

#include <Ice/SslSystemOpenSSL.h>
#include <string>

namespace IceSecurity
{

namespace Ssl
{

class GeneralConfig
{

public:
    GeneralConfig();

    inline SslProtocol getProtocol() const { return _sslVersion; };
    inline int getVerifyMode() const { return _verifyMode; };
    inline int getVerifyDepth() const { return _verifyDepth; };

    inline std::string getContext() const { return _context; };
    inline std::string getCipherList() const { return _cipherList; };
    inline std::string getRandomBytesFiles() const { return _randomBytesFiles; };

    // General method - it will figure out how to properly parse the data.
    void set(std::string&, std::string&);

protected:

    SslProtocol _sslVersion;

    int _verifyMode;
    int _verifyDepth;

    std::string _context;
    std::string _cipherList;
    std::string _randomBytesFiles;

    void parseVersion(std::string&);
    void parseVerifyMode(std::string&);
};

template<class Stream> inline
Stream& operator << (Stream& target, const GeneralConfig& generalConfig)
{
    target << "Protocol:     " << generalConfig.getProtocol() << std::endl;
    target << "Verify Mode:  " << generalConfig.getVerifyMode() << std::endl;
    target << "Verify Depth: " << generalConfig.getVerifyDepth() << std::endl;
    target << "Context:      " << generalConfig.getContext() << std::endl;
    target << "Cipher List:  " << generalConfig.getCipherList() << std::endl;
    target << "Random Bytes: " << generalConfig.getRandomBytesFiles() << std::endl;

    return target;
}

}

}

#endif
