// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef GLACIER_GLACIER_I_H
#define GLACIER_GLACIER_I_H

#include <Ice/Ice.h>
#include <Ice/RSACertificateGen.h>
#include <Glacier/Starter.h>

namespace Glacier
{

using IceSSL::OpenSSL::RSACertificateGenContext;
using IceSSL::OpenSSL::RSACertificateGen;

class StarterI : public Starter
{
public:

    StarterI(const Ice::CommunicatorPtr&, const PasswordVerifierPrx&);

    void destroy();

    RouterPrx startRouter(const std::string&,
			  const std::string&,
			  Ice::ByteSeq&,
			  Ice::ByteSeq&,
			  Ice::ByteSeq&,
			  const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
    Ice::LoggerPtr _logger;
    Ice::PropertiesPtr _properties;
    PasswordVerifierPrx _verifier;
    int _traceLevel;
    RSACertificateGenContext _certContext;
    RSACertificateGen _certificateGenerator;
};

class CryptPasswordVerifierI : public PasswordVerifier, public IceUtil::Mutex
{
public:

    CryptPasswordVerifierI(const std::map<std::string, std::string>&);

    virtual bool checkPassword(const std::string&, const std::string&, const Ice::Current&);
    virtual void destroy(const Ice::Current&);

private:

    const std::map<std::string, std::string> _passwords;
};

}

#endif
