// **********************************************************************
//
// Copyright (c) 2003
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
