// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_DH_PARAMS_H
#define ICE_DH_PARAMS_H

#include <IceUtil/Config.h>
#include <IceUtil/Shared.h>
#include <openssl/ssl.h>
#include <Ice/DHParamsF.h>

namespace IceSSL
{

namespace OpenSSL
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

}

#endif
