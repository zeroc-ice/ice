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

#ifndef ICE_DH_PARAMS_H
#define ICE_DH_PARAMS_H

#include <IceUtil/Shared.h>
#include <IceSSL/DHParamsF.h>
#include <openssl/ssl.h>

namespace IceSSL
{

class DHParams : public IceUtil::Shared
{
public:

    // Construction from DH Params structure (simple initialization).
    DHParams(DH*);

    ~DHParams();

    // Get the internal key structure as per the OpenSSL implementation.
    DH* get() const;

private:

    DH* _dhParams;
};

}

#endif
