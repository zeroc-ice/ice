// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceSSL/DHParams.h>

void IceInternal::incRef(::IceSSL::DHParams* p) { p->__incRef(); }
void IceInternal::decRef(::IceSSL::DHParams* p) { p->__decRef(); }

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

