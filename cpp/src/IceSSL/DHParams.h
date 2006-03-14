// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
