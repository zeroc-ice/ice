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

using std::string;
using std::ostream;

class GeneralConfig
{

public:
    GeneralConfig();

    inline SslProtocol getProtocol() const { return _sslVersion; };
    inline int getVerifyMode() const { return _verifyMode; };
    inline int getVerifyDepth() const { return _verifyDepth; };

    inline string getContext() const { return _context; };
    inline string getCipherList() const { return _cipherList; };
    inline string getRandomBytesFiles() const { return _randomBytesFiles; };

    // General method - it will figure out how to properly parse the data.
    void set(string&, string&);

protected:

    SslProtocol _sslVersion;

    int _verifyMode;
    int _verifyDepth;

    string _context;
    string _cipherList;
    string _randomBytesFiles;

    void parseVersion(string&);
    void parseVerifyMode(string&);
};

template<class Stream> inline
Stream& operator << (Stream& target, const GeneralConfig& generalConfig)
{
    target << "Protocol:     " << generalConfig.getProtocol() << endl;
    target << "Verify Mode:  " << generalConfig.getVerifyMode() << endl;
    target << "Verify Depth: " << generalConfig.getVerifyDepth() << endl;
    target << "Context:      " << generalConfig.getContext() << endl;
    target << "Cipher List:  " << generalConfig.getCipherList() << endl;
    target << "Random Bytes: " << generalConfig.getRandomBytesFiles() << endl;

    return target;
}

}

}

#endif
