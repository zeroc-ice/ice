//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Shared.h>

using namespace IceUtil;

//
// Flag constant used by the Shared class.
//
const unsigned char IceUtil::Shared::NoDelete = 1;

IceUtil::SimpleShared::SimpleShared() :
    _ref(0),
    _noDelete(false)
{
}

IceUtil::SimpleShared::SimpleShared(const SimpleShared&) :
    _ref(0),
    _noDelete(false)
{
}

IceUtil::SimpleShared::~SimpleShared()
{
    // Out of line to avoid weak vtable
}

IceUtil::Shared::Shared() :
    _ref(0),
    _flags(0)
{
}

IceUtil::Shared::Shared(const Shared&) :
    _ref(0),
    _flags(0)
{
}

void
IceUtil::Shared::__incRef()
{
    assert(_ref >= 0);
    ++_ref;
}

void
IceUtil::Shared::__decRef()
{
    assert(_ref > 0);
    if(--_ref == 0 && !(_flags & NoDelete))
    {
        delete this;
    }
}

int
IceUtil::Shared::__getRef() const
{
    return _ref;
}

void
IceUtil::Shared::__setNoDelete(bool b)
{
    _flags = b ? (_flags | NoDelete) : (_flags & ~NoDelete);
}
