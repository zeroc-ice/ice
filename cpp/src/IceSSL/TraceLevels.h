// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_TRACE_LEVELS_H
#define ICE_SSL_TRACE_LEVELS_H

#include <IceUtil/Shared.h>
#include <Ice/ProtocolPluginFacadeF.h>
#include <IceSSL/TraceLevelsF.h>

namespace IceSSL
{

class TraceLevels : public ::IceUtil::Shared
{
public:

    TraceLevels(const IceInternal::ProtocolPluginFacadePtr&);
    virtual ~TraceLevels();

    const int network;
    const char* networkCat;

    const int security;
    const char* securityCat;
};

}

#endif
