// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceSSL/DHParams.h>

void ::IceInternal::incRef(::IceSSL::DHParams* p) { p->__incRef(); }
void ::IceInternal::decRef(::IceSSL::DHParams* p) { p->__decRef(); }

IceSSL::DHParams::DHParams(DH* dhParams) :
    _dhParams(dhParams)
{
    assert(_dhParams != 0);
}

IceSSL::DHParams::~DHParams()
{
    if(_dhParams != 0)
    {
        DH_free(_dhParams);
    }
}

DH*
IceSSL::DHParams::get() const
{
    return _dhParams;
}

