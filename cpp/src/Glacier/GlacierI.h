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
#include <Ice/SslRSACertificateGen.h>
#include <Glacier/Glacier.h>

namespace Glacier
{

class StarterI : public Starter
{
public:

    StarterI(const Ice::CommunicatorPtr&);

    void destroy();

    Ice::RouterPrx startRouter(const std::string&, const std::string&, const Ice::Current&);

private:

    Ice::CommunicatorPtr _communicator;
    Ice::LoggerPtr _logger;
    Ice::PropertiesPtr _properties;
    int _traceLevel;
    RSACertificateGenContext _certContext;
    RSACertificateGen _certificateGenerator;
};

}

#endif
