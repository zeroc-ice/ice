// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_DH_PARAMS_H
#define ICE_DH_PARAMS_H

#include <IceUtil/Shared.h>
#include <IceSSL/DHParamsF.h>
#include <openssl/ssl.h>

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
