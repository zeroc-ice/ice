// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GLACIER_STARTER_I_H
#define GLACIER_STARTER_I_H

#include <Ice/Ice.h>
#include <IceSSL/RSACertificateGen.h>
#include <Glacier/Starter.h>

namespace Glacier
{

using IceSSL::RSACertificateGenContext;
using IceSSL::RSACertificateGen;

class StarterI : public Starter
{
public:

    StarterI(const Ice::CommunicatorPtr&, const PermissionsVerifierPrx&);

    void destroy();

    RouterPrx startRouter(const std::string&,
			  const std::string&,
			  Ice::ByteSeq&,
			  Ice::ByteSeq&,
			  Ice::ByteSeq&,
			  const Ice::Current&);

    virtual Ice::SliceChecksumDict getSliceChecksums(const Ice::Current&) const;

private:

    Ice::CommunicatorPtr _communicator;
    Ice::LoggerPtr _logger;
    Ice::PropertiesPtr _properties;
    PermissionsVerifierPrx _verifier;
    int _traceLevel;
    RSACertificateGenContext _certContext;
    RSACertificateGen _certificateGenerator;
};

typedef IceUtil::Handle<StarterI> StarterIPtr;

class CryptPasswordVerifierI : public PermissionsVerifier, public IceUtil::Mutex
{
public:

    CryptPasswordVerifierI(const std::map<std::string, std::string>&);

    virtual bool checkPermissions(const std::string&, const std::string&, std::string&, const Ice::Current&) const;
    virtual void destroy(const Ice::Current&);

private:

    const std::map<std::string, std::string> _passwords;
};

}

#endif
