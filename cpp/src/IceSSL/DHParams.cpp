// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceSSL/DHParams.h>

void ::IceInternal::incRef(::IceSSL::OpenSSL::DHParams* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSSL::OpenSSL::DHParams* p) { p->__decRef(); }

IceSSL::OpenSSL::DHParams::DHParams(DH* dhParams) :
                          _dhParams(dhParams)
{
    assert(_dhParams != 0);
}

IceSSL::OpenSSL::DHParams::~DHParams()
{
    if (_dhParams != 0)
    {
        DH_free(_dhParams);
    }
}

DH*
IceSSL::OpenSSL::DHParams::get() const
{
    return _dhParams;
}

