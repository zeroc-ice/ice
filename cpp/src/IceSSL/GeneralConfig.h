// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_SSL_GENERAL_CONFIG_H
#define ICE_SSL_GENERAL_CONFIG_H

#include <IceSSL/OpenSSL.h>

namespace IceSSL
{

class GeneralConfig
{
public:

    GeneralConfig();

    SslProtocol getProtocol() const;
    int getVerifyMode() const;
    int getVerifyDepth() const;

    std::string getContext() const;
    std::string getCipherList() const;
    std::string getRandomBytesFiles() const;

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
    target << "Protocol:     " << generalConfig.getProtocol() << "\n";
    target << "Verify Mode:  " << generalConfig.getVerifyMode() << "\n";
    target << "Verify Depth: " << generalConfig.getVerifyDepth() << "\n";
    target << "Context:      " << generalConfig.getContext() << "\n";
    target << "Cipher List:  " << generalConfig.getCipherList() << "\n";
    target << "Random Bytes: " << generalConfig.getRandomBytesFiles() << "\n";

    return target;
}

}

#endif
