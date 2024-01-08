//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#include "SynchronizationException.h"

#ifdef ICE_CPP11_MAPPING // C++11 mapping

IceGrid::SynchronizationException::~SynchronizationException()
{
}

const ::std::string&
IceGrid::SynchronizationException::ice_staticId()
{
    static const ::std::string typeId = "::IceGrid::SynchronizationException";
    return typeId;
}

#else

IceGrid::SynchronizationException::SynchronizationException(const char* file, int line) :
    ::Ice::LocalException(file, line)
{
}

#ifdef ICE_CPP11_COMPILER
IceGrid::SynchronizationException::~SynchronizationException()
{
}
#else
IceGrid::SynchronizationException::~SynchronizationException() throw()
{
}
#endif

::std::string
IceGrid::SynchronizationException::ice_id() const
{
    return "::IceGrid::SynchronizationException";
}

IceGrid::SynchronizationException*
IceGrid::SynchronizationException::ice_clone() const
{
    return new SynchronizationException(*this);
}

void
IceGrid::SynchronizationException::ice_throw() const
{
    throw *this;
}

#endif
