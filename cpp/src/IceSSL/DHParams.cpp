// **********************************************************************
//
// Copyright (c) 2002
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

